#include "cli.h"
#include "led.h"

#define CLI_LINE_BUF_MAX 64
#define CLI_CMD_LIST_MAX 32
#define CLI_CMD_ARG_MAX 4

typedef struct {
    char cmd_str[16];
    void (*cmd_func)(uint8_t argc, char *argv[]);
} cli_cmd_t;

static cli_cmd_t cli_cmd_list[CLI_CMD_LIST_MAX];
static uint8_t cli_cmd_cnt = 0;

static uint8_t cli_argc = 0;
static char *cli_argv[CLI_CMD_ARG_MAX];
static char cli_line_buf[CLI_LINE_BUF_MAX];
static uint16_t cli_line_buf_idx = 0;

#define CLI_HISTORY_BUF_MAX 10
static char cli_history_buf[CLI_HISTORY_BUF_MAX][CLI_LINE_BUF_MAX];
static uint8_t cli_history_cnt = 0;
static uint8_t cli_history_idx = 0;
static uint8_t cli_history_depth = 0;

typedef enum { CLI_STATE_NORMAL = 0, CLI_STATE_ESC_RCVD, CLI_STATE_BRAKCET_RCVD } cli_input_state_t;

static cli_input_state_t cli_input_state = CLI_STATE_NORMAL;

static void cliLed(uint8_t argc, char *argv[]);
static bool cliLedColorFromString(const char *str, LED_COLOR *color);
static bool cliIsLedAlias(const char *cmd_str);

static void handleEnterKey(void)
{
    cliPrintf("\r\n");
    cli_line_buf[cli_line_buf_idx] = '\0';

    strncpy(cli_history_buf[cli_history_cnt], cli_line_buf, CLI_LINE_BUF_MAX);
    cli_history_cnt = (cli_history_cnt + 1) % CLI_HISTORY_BUF_MAX;
    cli_history_idx = cli_history_cnt;
    cli_history_depth =
        cli_history_depth < CLI_HISTORY_BUF_MAX ? cli_history_depth + 1 : CLI_HISTORY_BUF_MAX;

    cliParseArgs(cli_line_buf);
    cliRunCommand();

    cli_line_buf_idx = 0;
    cliPrintf("CLI> ");
}

static void handleBackspace(void)
{
    if (cli_line_buf_idx > 0) {
        cli_line_buf_idx--;
        cliPrintf("\b \b");
    }
}

static void handleCharInsert(uint8_t c)
{
    cliPrintf("%c", c);
    cli_line_buf[cli_line_buf_idx++] = c;
    if (cli_line_buf_idx >= CLI_LINE_BUF_MAX) {
        cli_line_buf_idx = 0;
    }
}

static void handleArrowKeys(uint8_t rx_data)
{
    if (rx_data == 'A') {
        for (uint16_t i = 0; i < cli_line_buf_idx; i++) {
            cliPrintf("\b \b");
        }

        if (cli_history_idx == 0 && cli_history_depth == CLI_HISTORY_BUF_MAX) {
            cli_history_idx = CLI_HISTORY_BUF_MAX - 1;
        } else if (cli_history_idx != 0) {
            if (cli_history_idx != cli_history_cnt + 1) {
                cli_history_idx--;
            }
        }
        strncpy(cli_line_buf, cli_history_buf[cli_history_idx], CLI_LINE_BUF_MAX);

        cli_line_buf_idx = strlen(cli_line_buf);
        cliPrintf("%s", cli_line_buf);
    } else if (rx_data == 'B' && cli_history_idx != cli_history_cnt) {
        for (uint16_t i = 0; i < cli_line_buf_idx; i++) {
            cliPrintf("\b \b");
        }

        if (cli_history_idx == CLI_HISTORY_BUF_MAX - 1 &&
            cli_history_depth == CLI_HISTORY_BUF_MAX) {
            cli_history_idx = 0;
        } else if (cli_history_idx != CLI_HISTORY_BUF_MAX - 1) {
            if (cli_history_idx != cli_history_cnt - 1) {
                cli_history_idx++;
            }
        }
        strncpy(cli_line_buf, cli_history_buf[cli_history_idx], CLI_LINE_BUF_MAX);

        cli_line_buf_idx = strlen(cli_line_buf);
        cliPrintf("%s", cli_line_buf);
    }
}

static void processAnsiEscape(uint8_t rx_data)
{
    switch (rx_data) {
    case 0x1B:
        cli_input_state = CLI_STATE_ESC_RCVD;
        break;
    case '[':
        if (cli_input_state == CLI_STATE_ESC_RCVD) {
            cli_input_state = CLI_STATE_BRAKCET_RCVD;
        } else {
            cli_input_state = CLI_STATE_NORMAL;
        }
        break;
    case 'A':
    case 'B':
        if (cli_input_state == CLI_STATE_BRAKCET_RCVD) {
            handleArrowKeys(rx_data);
        }
        cli_input_state = CLI_STATE_NORMAL;
        break;
    default:
        cli_input_state = CLI_STATE_NORMAL;
        break;
    }
}

static void cliInfo(uint8_t argc, char *argv[])
{
    if (argc == 1) {
        cliPrintf("=====================================\r\n");
        cliPrintf("  HW Model    : STM32F411\r\n");
        cliPrintf("  FW Version  : V1.0.0\r\n");
        cliPrintf("  Build Date  : %s %s\r\n", __DATE__, __TIME__);

        uint32_t hal = HAL_GetHalVersion();
        uint32_t rev = HAL_GetREVID();
        uint32_t dev = HAL_GetDEVID();
        uint32_t uid0 = HAL_GetUIDw0();
        uint32_t uid1 = HAL_GetUIDw1();
        uint32_t uid2 = HAL_GetUIDw2();

        cliPrintf("  HAL Version : %d.%d.%d\r\n", (hal >> 24) & 0xFF, (hal >> 16) & 0xFF,
                  hal & 0xFFFF);
        cliPrintf("  Device ID   : %08X\r\n", dev);
        cliPrintf("  Revision ID : %08X\r\n", rev);
        cliPrintf("  Serial Num  : %08X-%08X-%08X\r\n", uid0, uid1, uid2);
        cliPrintf("=====================================\r\n");
    } else if (argc == 2 && strcmp(argv[1], "uptime") == 0) {
        cliPrintf("System Uptime: %d ms \r\n", millis());
    } else {
        cliPrintf("Usage: info [uptime]\r\n");
    }
}

static void cliSys(uint8_t argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "reset") == 0) {
        cliPrintf("System Resetting...\r\n");
        NVIC_SystemReset();
    } else {
        cliPrintf("Usage: sys [reset]\r\n");
    }
}

static void cliHelp(uint8_t argc, char *argv[])
{
    cliPrintf("----------CLI Commands----------\r\n");

    for (uint8_t i = 0; i < cli_cmd_cnt; i++) {
        cliPrintf("%s\r\n", cli_cmd_list[i].cmd_str);
    }
    cliPrintf("--------------------------------\r\n");
}

static void cliClear(uint8_t argc, char *argv[])
{
    cliPrintf("\x1B[2J\x1B[H");
}

static void cliDoor(uint8_t argc, char *argv[])
{
    int id;

    if (argc == 1)
    {
        cliPrintf("Usage:\r\n");
        cliPrintf("  door [0~3] open/close/status\r\n");
        cliPrintf("  door all open/close\r\n");
        return;
    }

    // ----------- ALL 제어 -----------
    if (strcmp(argv[1], "all") == 0)
    {
        if (argc != 3)
        {
            cliPrintf("Usage: door all open/close\r\n");
            return;
        }

        if (strcmp(argv[2], "open") == 0)
        {
            for (int i = 0; i < SERVO_COUNT; i++)
            {
                changeDoorState(i, DOOR_OPEN);
            }
            cliPrintf("all doors open\r\n");
        }
        else if (strcmp(argv[2], "close") == 0)
        {
            for (int i = 0; i < SERVO_COUNT; i++)
            {
                changeDoorState(i, DOOR_CLOSE);
            }
            cliPrintf("all doors close\r\n");
        }
        else
        {
            cliPrintf("Usage: door all open/close\r\n");
        }
        return;
    }
    // ----------- 전체 상태 조회 -----------
    if (argc == 2 && strcmp(argv[1], "status") == 0)
    {
        for (int i = 0; i < SERVO_COUNT; i++)
        {
            cliPrintf("door %d is %s\r\n",
                    i,
                    getDoorState(i) == DOOR_OPEN ? "open" : "close");
        }
        return;
    }

  
    // ----------- 개별 문 제어 -----------
    if (strcmp(argv[1], "0") == 0)
    {
        id = 0;
    }
    else if (strcmp(argv[1], "1") == 0)
    {
        id = 1;
    }
    else if (strcmp(argv[1], "2") == 0)
    {
        id = 2;
    }
    else if (strcmp(argv[1], "3") == 0)
    {
        id = 3;
    }
    else
    {
        cliPrintf("Usage:\r\n");
        cliPrintf("  door [0~3] open/close/status\r\n");
        cliPrintf("  door all open/close\r\n");
        cliPrintf("  door status\r\n");
        return;
    }
    // ----------- 상태 조회 -----------
    if (argc == 2 || strcmp(argv[2], "status") == 0)
    {
        cliPrintf("door %d is %s\r\n",
                  id,
                  getDoorState(id) == DOOR_OPEN ? "open" : "close");
        return;
    }

    // ----------- OPEN -----------
    if (strcmp(argv[2], "open") == 0)
    {
        if (getDoorState(id) == DOOR_OPEN)
        {
            cliPrintf("door %d already open\r\n", id);
            return;
        }

        changeDoorState(id, DOOR_OPEN);
        cliPrintf("door %d open\r\n", id);
    }
    // ----------- CLOSE -----------
    else if (strcmp(argv[2], "close") == 0)
    {
        if (getDoorState(id) == DOOR_CLOSE)
        {
            cliPrintf("door %d already close\r\n", id);
            return;
        }

        changeDoorState(id, DOOR_CLOSE);
        cliPrintf("door %d close\r\n", id);
    }
    else
    {
        cliPrintf("Usage:\r\n");
        cliPrintf("  door [0~3] open/close/status\r\n");
        cliPrintf("  door all open/close\r\n");
    }
}

static void cliIr(uint8_t argc, char *argv[])
{
    uint32_t period;

    if (argc == 1)
    {
        cliPrintf("Usage:\r\n");
        cliPrintf("  ir on [period_ms]\r\n");
        cliPrintf("  ir off\r\n");
        cliPrintf("  ir read\r\n");
        return;
    }

    // ----------- 현재 값 1번 읽기 -----------
    if (strcmp(argv[1], "read") == 0)
    {
        irSensorPrintValue();
        return;
    }

    // ----------- OFF -----------
    if (strcmp(argv[1], "off") == 0)
    {
        if (irSensorGetMonitor() == IR_MON_OFF)
        {
            cliPrintf("ir already off\r\n");
            return;
        }

        irSensorSetMonitor(IR_MON_OFF);
        cliPrintf("ir off\r\n");
        return;
    }

    // ----------- ON period 설정 -----------
    if (strcmp(argv[1], "on") == 0)
    {
        if (argc != 3)
        {
            cliPrintf("Usage: ir on [period_ms]\r\n");
            return;
        }

        period = atoi(argv[2]);

        if (period == 0)
        {
            cliPrintf("invalid period\r\n");
            return;
        }

        irSensorSetPeriod(period);
        irSensorSetMonitor(IR_MON_ON);

        cliPrintf("ir on %dms\r\n", period);
        return;
    }

    cliPrintf("Usage:\r\n");
    cliPrintf("  ir on [period_ms]\r\n");
    cliPrintf("  ir off\r\n");
    cliPrintf("  ir read\r\n");
}

void cliInit(void)
{
    cli_cmd_cnt = 0;
    cli_argc = 0;
    cli_line_buf_idx = 0;

    cliClear(0, NULL);

    LOG_INF("CLI Init");

    cliAdd("help", cliHelp);
    cliAdd("cls", cliClear);
    cliAdd("info", cliInfo);
    cliAdd("sys", cliSys);
    cliAdd("log", cliLog);
    cliAdd("dht", cliDht);
    cliAdd("door", cliDoor);
    cliAdd("led", cliLed);
    cliAdd("ir", cliIr);
}

void cliRunCommand(void)
{
    if (cli_argc == 0) {
        return;
    }

    bool is_found = false;
    for (uint8_t i = 0; i < cli_cmd_cnt; i++) {
        if (strcmp(cli_argv[0], cli_cmd_list[i].cmd_str) == 0 ||
            (strcmp(cli_cmd_list[i].cmd_str, "led") == 0 && cliIsLedAlias(cli_argv[0]))) {
            cli_cmd_list[i].cmd_func(cli_argc, cli_argv);
            is_found = true;
            break;
        }
    }

    if (!is_found) {
        cliPrintf("Command Not Found\r\n");
    }
}

void cliPrintf(const char *fmt, ...)
{
    char buf[CLI_LINE_BUF_MAX];
    uint32_t len;
    va_list args;

    va_start(args, fmt);

    len = vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);
    uartWrite(0, (uint8_t *)buf, len);
}

void cliParseArgs(char *line_buf)
{
    char *token;
    cli_argc = 0;
    token = strtok(line_buf, " ");
    while (token != NULL && cli_argc < CLI_CMD_ARG_MAX) {
        cli_argv[cli_argc++] = token;
        token = strtok(NULL, " ");
    }
}

bool cliAdd(const char *cmd_str, void (*cmd_func)(uint8_t argc, char *argv[]))
{
    if (cli_cmd_cnt >= CLI_CMD_LIST_MAX) {
        return false;
    }

    strncpy(cli_cmd_list[cli_cmd_cnt].cmd_str, cmd_str, strlen(cmd_str) + 1);
    cli_cmd_list[cli_cmd_cnt].cmd_func = cmd_func;
    cli_cmd_cnt++;

    return true;
}

static cli_callback_t ctrl_c_handler = NULL;

void cliSetCtrlCHandler(cli_callback_t handler)
{
    ctrl_c_handler = handler;
}

void cliMain(void)
{
    uint8_t rx_data;
    if (uartReadBlock(0, &rx_data, 0) == false) {
        return;
    }

    if (cli_input_state != CLI_STATE_NORMAL) {
        processAnsiEscape(rx_data);
        return;
    }

    switch (rx_data) {
    case 0x03:
        if (ctrl_c_handler != NULL) {
            ctrl_c_handler();
        }
        cliPrintf("^c \r\nCLI>");
        cli_line_buf_idx = 0;
        break;
    case 0x1B:
        cli_input_state = CLI_STATE_ESC_RCVD;
        break;
    case '\r':
    case '\n':
        handleEnterKey();
        break;
    case '\b':
    case 127:
        handleBackspace();
        break;
    default:
        if (32 <= rx_data && rx_data <= 126)
            handleCharInsert(rx_data);
        break;
    }
}

static void cliLed(uint8_t argc, char *argv[])
{
    LED_COLOR color;
    uint8_t id;

    if (argc == 2 &&
        strncmp(argv[0], "led", 3) == 0 &&
        argv[0][3] >= '1' && argv[0][3] <= '3' &&
        argv[0][4] == '\0') {
        id = (uint8_t)(argv[0][3] - '1');

        if (!cliLedColorFromString(argv[1], &color)) {
            cliPrintf("Usage: led[1|2|3] [yellow|white|warm|off]\r\n");
            return;
        }
    } else {
        cliPrintf("Usage: led[1|2|3] [yellow|white|warm|off]\r\n");
        return;
    }

    setLedColor(id, color);
    cliPrintf("LED%u ok\r\n", id + 1);
}

static bool cliLedColorFromString(const char *str, LED_COLOR *color)
{
    if (strcmp(str, "off") == 0) {
        *color = LED_OFF;
    } else if (strcmp(str, "yellow") == 0) {
        *color = LED_YELLOW;
    } else if (strcmp(str, "white") == 0) {
        *color = LED_WHITE;
    } else if (strcmp(str, "warm") == 0) {
        *color = LED_WARM_WHITE;
    } else {
        return false;
    }

    return true;
}

static bool cliIsLedAlias(const char *cmd_str)
{
    return strncmp(cmd_str, "led", 3) == 0 &&
           cmd_str[3] >= '1' && cmd_str[3] <= '3' &&
           cmd_str[4] == '\0';
}
