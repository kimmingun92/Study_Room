#include "my_rfid.h"

/* RC522 레지스터 */
#define REG_COMMAND        0x01
#define REG_COM_IEN        0x02
#define REG_COM_IRQ        0x04
#define REG_ERROR          0x06
#define REG_FIFO_DATA      0x09
#define REG_FIFO_LEVEL     0x0A
#define REG_CONTROL        0x0C
#define REG_BIT_FRAMING    0x0D
#define REG_MODE           0x11
#define REG_TX_CONTROL     0x14
#define REG_TX_ASK         0x15
#define REG_T_MODE         0x2A
#define REG_T_PRESCALER    0x2B
#define REG_T_RELOAD_H     0x2C
#define REG_T_RELOAD_L     0x2D
#define REG_VERSION        0x37

#define CMD_IDLE           0x00
#define CMD_TRANSCEIVE     0x0C
#define CMD_SOFT_RESET     0x0F

#define PICC_REQA          0x26
#define PICC_ANTICOLL      0x93

#define RFID_DOOR_COUNT    3
#define RFID_OPEN_TIME_MS  5000
#define RFID_SCAN_DELAY_MS 1000

typedef struct
{
    uint8_t uid[4];
    uint8_t door_id;
} RFID_User_t;

/* 등록 카드 UID */
static RFID_User_t rfid_user_tbl[] =
{
    { {0xED,0x89,0x6D,0x05}, 0 }, // 0번 방
    { {0xCC,0xEF,0x4B,0x06}, 1 }, // 1번 방
    { {0x11,0x22,0x33,0x44}, 2 }, // 수정 필요
};

static uint32_t door_close_time[RFID_DOOR_COUNT] = {0, };
static uint32_t last_scan_time = 0;
static bool rfid_monitor = false;

/* CS 제어 */
static void RFID_CS_LOW(void)
{
    HAL_GPIO_WritePin(RFID_CS_GPIO_Port, RFID_CS_Pin, GPIO_PIN_RESET);
}

static void RFID_CS_HIGH(void)
{
    HAL_GPIO_WritePin(RFID_CS_GPIO_Port, RFID_CS_Pin, GPIO_PIN_SET);
}

/* RC522 레지스터 접근 */
static void rc522WriteReg(uint8_t reg, uint8_t value)
{
    RFID_CS_LOW();
    spiTransferByte((reg << 1) & 0x7E);
    spiTransferByte(value);
    RFID_CS_HIGH();
}

static uint8_t rc522ReadReg(uint8_t reg)
{
    uint8_t value;

    RFID_CS_LOW();
    spiTransferByte(((reg << 1) & 0x7E) | 0x80);
    value = spiTransferByte(0xFF);
    RFID_CS_HIGH();

    return value;
}

static void rc522SetBits(uint8_t reg, uint8_t mask)
{
    rc522WriteReg(reg, rc522ReadReg(reg) | mask);
}

static void rc522ClearBits(uint8_t reg, uint8_t mask)
{
    rc522WriteReg(reg, rc522ReadReg(reg) & ~mask);
}

static void rc522Reset(void)
{
    rc522WriteReg(REG_COMMAND, CMD_SOFT_RESET);
    delay(50);
}

static void rc522AntennaOn(void)
{
    uint8_t temp = rc522ReadReg(REG_TX_CONTROL);

    if ((temp & 0x03) != 0x03)
    {
        rc522SetBits(REG_TX_CONTROL, 0x03);
    }
}

/* 초기화 */
bool rfidInit(void)
{
    spiInit();

    RFID_CS_HIGH();

    HAL_GPIO_WritePin(RFID_RST_GPIO_Port, RFID_RST_Pin, GPIO_PIN_RESET);
    delay(10);
    HAL_GPIO_WritePin(RFID_RST_GPIO_Port, RFID_RST_Pin, GPIO_PIN_SET);
    delay(50);

    rc522Reset();

    rc522WriteReg(REG_T_MODE,      0x8D);
    rc522WriteReg(REG_T_PRESCALER, 0x3E);
    rc522WriteReg(REG_T_RELOAD_L,  30);
    rc522WriteReg(REG_T_RELOAD_H,  0);
    rc522WriteReg(REG_TX_ASK,      0x40);
    rc522WriteReg(REG_MODE,        0x3D);

    rc522AntennaOn();

    return true;
}

uint8_t rfidGetVersion(void)
{
    return rc522ReadReg(REG_VERSION);
}

/* 카드 통신 핵심 */
static rfid_status_t rc522ToCard(uint8_t cmd,
                                 uint8_t *send_data,
                                 uint8_t send_len,
                                 uint8_t *back_data,
                                 uint16_t *back_len)
{
    rfid_status_t status = RFID_ERROR;
    uint8_t irq_en = 0x00;
    uint8_t wait_irq = 0x00;
    uint8_t n;
    uint16_t i;

    if (cmd == CMD_TRANSCEIVE)
    {
        irq_en = 0x77;
        wait_irq = 0x30;
    }

    rc522WriteReg(REG_COM_IEN, irq_en | 0x80);
    rc522ClearBits(REG_COM_IRQ, 0x80);
    rc522SetBits(REG_FIFO_LEVEL, 0x80);
    rc522WriteReg(REG_COMMAND, CMD_IDLE);

    RFID_CS_LOW();
    spiTransferByte((REG_FIFO_DATA << 1) & 0x7E);

    for (uint8_t k = 0; k < send_len; k++)
    {
        spiTransferByte(send_data[k]);
    }

    RFID_CS_HIGH();

    rc522WriteReg(REG_COMMAND, cmd);

    if (cmd == CMD_TRANSCEIVE)
    {
        rc522SetBits(REG_BIT_FRAMING, 0x80);
    }

    i = 50000;

    do
    {
        n = rc522ReadReg(REG_COM_IRQ);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & wait_irq));

    rc522ClearBits(REG_BIT_FRAMING, 0x80);

    if (i == 0)
        return RFID_TIMEOUT;

    if (rc522ReadReg(REG_ERROR) & 0x1B)
        return RFID_ERROR;

    if (n & wait_irq)
    {
        uint8_t fifo_n = rc522ReadReg(REG_FIFO_LEVEL);
        uint8_t last_bits = rc522ReadReg(REG_CONTROL) & 0x07;

        if (fifo_n == 0)
            return RFID_ERROR;

        if (last_bits)
            *back_len = (fifo_n - 1) * 8 + last_bits;
        else
            *back_len = fifo_n * 8;

        for (uint8_t j = 0; j < fifo_n; j++)
        {
            back_data[j] = rc522ReadReg(REG_FIFO_DATA);
        }

        status = RFID_OK;
    }

    return status;
}

/* UID 읽기 */
rfid_status_t rfidReadUid(rfid_uid_t *uid)
{
    uint8_t tag_type[2];
    uint8_t serial_buf[2];
    uint8_t back_data[8] = {0, };
    uint16_t back_bits = 0;
    uint8_t serial_check = 0;

    if (uid == NULL)
        return RFID_ERROR;

    rc522WriteReg(REG_BIT_FRAMING, 0x07);

    tag_type[0] = PICC_REQA;

    if (rc522ToCard(CMD_TRANSCEIVE, tag_type, 1, tag_type, &back_bits) != RFID_OK)
        return RFID_NOTAG;

    if (back_bits != 0x10)
        return RFID_NOTAG;

    rc522WriteReg(REG_BIT_FRAMING, 0x00);

    serial_buf[0] = PICC_ANTICOLL;
    serial_buf[1] = 0x20;

    if (rc522ToCard(CMD_TRANSCEIVE, serial_buf, 2, back_data, &back_bits) != RFID_OK)
        return RFID_NOTAG;

    for (uint8_t i = 0; i < 4; i++)
    {
        serial_check ^= back_data[i];
    }

    if (serial_check != back_data[4])
        return RFID_ERROR;

    uid->size = 4;

    for (uint8_t i = 0; i < 4; i++)
    {
        uid->bytes[i] = back_data[i];
    }

    return RFID_OK;
}

/* UID → door 매칭 */
static int8_t rfidFindDoor(rfid_uid_t *uid)
{
    uint8_t count = sizeof(rfid_user_tbl) / sizeof(rfid_user_tbl[0]);

    if (uid == NULL || uid->size != 4)
        return -1;

    for (uint8_t i = 0; i < count; i++)
    {
        bool match = true;

        for (uint8_t j = 0; j < 4; j++)
        {
            if (uid->bytes[j] != rfid_user_tbl[i].uid[j])
            {
                match = false;
                break;
            }
        }

        if (match)
            return rfid_user_tbl[i].door_id;
    }

    return -1;
}

/* monitor 제어 */
void rfidSetMonitor(bool state)
{
    rfid_monitor = state;
}

bool rfidGetMonitor(void)
{
    return rfid_monitor;
}

/* RFID 처리 */
void rfidProcess(void)
{
    rfid_uid_t uid;
    uint32_t now = millis();

    /* 15초 지난 문 자동 닫기 */
    for (uint8_t i = 0; i < RFID_DOOR_COUNT; i++)
    {
        if (door_close_time[i] != 0 && now >= door_close_time[i])
        {
            changeDoorState(i, DOOR_CLOSE);
            door_close_time[i] = 0;
        }
    }

    /* rfid read 명령 전에는 감시 안 함 */
    if (!rfid_monitor)
        return;

    /* 너무 자주 읽지 않게 제한 */
    if (now - last_scan_time < RFID_SCAN_DELAY_MS)
        return;

    last_scan_time = now;

    if (rfidReadUid(&uid) == RFID_OK)
    {
        int8_t door_id = rfidFindDoor(&uid);

        if (door_id >= 0 && door_id < RFID_DOOR_COUNT)
        {
            changeDoorState(door_id, DOOR_OPEN);
            door_close_time[door_id] = now + RFID_OPEN_TIME_MS;
        }
    }
}