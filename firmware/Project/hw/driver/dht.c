#include "dht.h"


#define DHT_PORT GPIOA
#define DHT_PIN GPIO_PIN_3

static uint8_t dht_tem = 0;
static uint8_t dht_hum = 0;
static volatile bool dht_print_on = false;
static volatile uint32_t dht_interval_ms = 2000;
static volatile uint32_t dht_prev_time = 0;
bool isAutoMotor = true;

static volatile uint32_t last_edge_time = 0;
static volatile int bit_index = -1;
static uint8_t raw_data[5] = {0};
static volatile bool dht_done = false;

extern TIM_HandleTypeDef htim2;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // 인터럽트 pin에 따른 문제로 ir Receiver 인터럽트 여기서 호출
    if(GPIO_Pin == IR_RX_PIN){
        irReceiverExtiCallback(GPIO_Pin);
    }
    if (GPIO_Pin == DHT_PIN) {
        uint32_t current_time = __HAL_TIM_GET_COUNTER(&htim2);
        uint32_t duration = current_time - last_edge_time;
        last_edge_time = current_time;

        // 하강 엣지에서 데이터 판별 (DHT는 데이터 시작 후 High 유지 시간으로 0/1 결정)
        if (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_RESET) {
            if (bit_index >= 0 && bit_index < 40) {
                raw_data[bit_index / 8] <<= 1;
                if (duration > 40) { // High 유지 시간이 길면 1 (보통 70us)
                    raw_data[bit_index / 8] |= 1;
                }
                bit_index++;
            } else if (bit_index == -1) {
                // DHT 응답 신호(80us Low + 80us High) 이후 첫 데이터 비트 준비
                bit_index = 0;
            }
        }
        
        if (bit_index == 40) {
            dht_done = true;
            // 모든 비트를 다 읽었으므로 인터럽트 잠시 비활성화 (필요 시)
        }
    }
}

static void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us) {
    }
}

static void dhtSetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static void dhtSetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static bool waitPin(GPIO_PinState state, uint32_t timeout_us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) != state) {
        if (__HAL_TIM_GET_COUNTER(&htim2) > timeout_us) {
            return false;
        }
    }

    return true;
}

bool dhtInit(void)
{
    HAL_TIM_Base_Start(&htim2);
    dhtSetInput();

    return true;
}

bool dhtRead(void) {
    bit_index = -2; // 초기 상태 (Response 대기)
    dht_done = false;
    memset(raw_data, 0, 5);

    // 1. 트리거 신호 (이 부분은 여전히 정밀 타이밍이 필요)
    dhtSetOutput();
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    HAL_Delay(18); // DHT11 규격에 맞게 18ms 대기 (또는 코드에 따라 조정)
    
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
    delay_us(30);

    // 2. 입력 모드 및 인터럽트 활성화
    dhtSetInput(); 
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    last_edge_time = 0;
    bit_index = -1; // 응답 신호 체크용 상태

    // 3. 데이터 수신 대기 (Timeout 설정)
    uint32_t timeout = HAL_GetTick();
    while (!dht_done) {
        if (HAL_GetTick() - timeout > 100) { // 100ms 타임아웃
            return false; 
        }
    }

    // 4. 체크섬 검사 (기존 로직 동일)
    uint8_t checksum = raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3];
    if (checksum != raw_data[4]) return false;

    uint16_t raw_hum = (raw_data[0] << 8) | raw_data[1];
    uint16_t raw_tem = (raw_data[2] << 8) | raw_data[3];

    dht_hum = raw_hum / 10;

    if (raw_tem & 0x8000)
    {
        raw_tem &= 0x7FFF;
        dht_tem = raw_tem / 10;
    }
    else
    {
        dht_tem = raw_tem / 10;
    }

    return true;
}

void cliDht(uint8_t argc, char *argv[])
{
    if (argc == 1) {
        dht_interval_ms = 2000;
        dht_print_on = true;
        dht_prev_time = millis() - dht_interval_ms;
        cliPrintf("dht on %dms\r\n", dht_interval_ms);
    } else if (argc == 2 && strcmp(argv[1], "off") == 0) {
        dht_print_on = false;
        cliPrintf("dht off\r\n");
    } else if (argc == 2 && strcmp(argv[1], "status") == 0) {
        cliPrintf("dht %s %dms\r\n", dht_print_on ? "on" : "off", dht_interval_ms);
        cliPrintf("range T:%d~%d H:%d~%d\r\n", motorR300GetTempMin(), motorR300GetTempMax(),
                  motorR300GetHumMin(), motorR300GetHumMax());
        cliPrintf("motor %s pulse off:%d on:%d\r\n", motorR300IsOn() ? "on" : "off",
                  motorR300GetOffPulse(), motorR300GetOnPulse());
    } else if (argc == 2) {
        uint32_t interval_ms = (uint32_t)atoi(argv[1]);

        if (interval_ms < 2000) {
            cliPrintf("Usage: dht [period_ms >= 2000]\r\n");
            cliPrintf("       dht off\r\n");
            return;
        }

        dht_interval_ms = interval_ms;
        dht_print_on = true;
        dht_prev_time = millis() - dht_interval_ms;
        cliPrintf("dht on %dms\r\n", dht_interval_ms);
    } else {
        cliPrintf("Usage: dht [period_ms >= 2000]\r\n");
        cliPrintf("       dht status\r\n");
        cliPrintf("       dht off\r\n");
    }
}

void dhtMain(void)
{
    if (millis() - dht_prev_time >= dht_interval_ms) {
        dht_prev_time = millis();

        if (dhtRead()) {
            if (isAutoMotor) {
                motorR300Update(getTem(), getHum());
            }

            if (dht_print_on == true) {
                cliPrintf("$%d,%d#\r\n", getTem(), getHum());
            }
        } else {
            if (dht_print_on == true) {
                cliPrintf("$Error#\r\n");
            }
        }
    }
}

uint8_t getTem(void)
{
    return dht_tem;
}

uint8_t getHum(void)
{
    return dht_hum;
}
