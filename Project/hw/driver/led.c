#include "led.h"
#include "pwm.h"

#define LED_COUNT             3
#define LED_PWM_CH_PER_LED    3

typedef struct {
    GPIO_TypeDef *r_port;
    uint16_t r_pin;
    GPIO_TypeDef *g_port;
    uint16_t g_pin;
    GPIO_TypeDef *b_port;
    uint16_t b_pin;
} led_pin_t;

static const led_pin_t led_tbl[LED_COUNT] = {
    /* LED1: D2 PF15(R), D3 PE13(G), D4 PF14(B) */
    {GPIOF, GPIO_PIN_15, GPIOE, GPIO_PIN_13, GPIOF, GPIO_PIN_14},
    /* LED2: D5 PE11(R), D6 PE9(G), D7 PF13(B) */
    {GPIOE, GPIO_PIN_11, GPIOE, GPIO_PIN_9, GPIOF, GPIO_PIN_13},
    /* LED3: D8 PF12(R), D9 PD15(G), D10 PD14(B) */
    {GPIOF, GPIO_PIN_12, GPIOD, GPIO_PIN_15, GPIOD, GPIO_PIN_14},
};

static LED_COLOR led_state[LED_COUNT];
static uint8_t led_power[LED_COUNT] = {100, 100, 100};

static void ledGpioInit(void);
static void ledProcessCommand(const char *cmd);
static void ledSetRgb(uint8_t id, uint8_t red, uint8_t green, uint8_t blue);
static uint8_t ledPwmChannel(uint8_t id, uint8_t color_index);
static bool ledParseCommand(const char *cmd, uint8_t *id, LED_COLOR *color);
static bool ledReadToken(const char **cmd, char *token, uint8_t token_size);
static bool ledStringEqual(const char *a, const char *b);
static bool ledColorFromString(const char *str, LED_COLOR *color);

void ledInit(void)
{
    ledGpioInit();
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        pwmGpioAttach(ledPwmChannel(i, 0), led_tbl[i].r_port, led_tbl[i].r_pin);
        pwmGpioAttach(ledPwmChannel(i, 1), led_tbl[i].g_port, led_tbl[i].g_pin);
        pwmGpioAttach(ledPwmChannel(i, 2), led_tbl[i].b_port, led_tbl[i].b_pin);
        setLedColor(i, LED_OFF);
    }
}

void ledUpdate(void)
{
}

void ledCommandProcess(const char *cmd)
{
    if (cmd == NULL) {
        return;
    }

    ledProcessCommand(cmd);
}

void setLedColor(uint8_t id, LED_COLOR color)
{
    if (id >= LED_COUNT) {
        return;
    }

    led_state[id] = color;

    switch (color) {
        case LED_YELLOW:
            ledSetRgb(id, 100, 35, 0);
            break;
        case LED_WHITE:
            ledSetRgb(id, 100, 65, 45);
            break;
        case LED_WARM_WHITE:
            ledSetRgb(id, 100, 62, 24);
            break;
        case LED_OFF:
        default:
            ledSetRgb(id, 0, 0, 0);
            break;
    }
}

LED_COLOR getLedColor(uint8_t id)
{
    if (id >= LED_COUNT) {
        return LED_OFF;
    }

    return led_state[id];
}

void setLedPower(uint8_t id, uint8_t brightness)
{
    if (id >= LED_COUNT) {
        return;
    }

    if (brightness > 100) {
        brightness = 100;
    }

    led_power[id] = brightness;
    setLedColor(id, led_state[id]);
}

uint8_t getLedPower(uint8_t id)
{
    if (id >= LED_COUNT) {
        return 0;
    }

    return led_power[id];
}

static void ledGpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

static void ledProcessCommand(const char *cmd)
{
    LED_COLOR color;
    uint8_t id;

    if (!ledParseCommand(cmd, &id, &color)) {
        return;
    }

    setLedColor(id, color);
}

static void ledSetRgb(uint8_t id, uint8_t red, uint8_t green, uint8_t blue)
{
    if (id >= LED_COUNT) {
        return;
    }

    pwmGpioWrite(ledPwmChannel(id, 0), (uint8_t)((red * led_power[id]) / 100));
    pwmGpioWrite(ledPwmChannel(id, 1), (uint8_t)((green * led_power[id]) / 100));
    pwmGpioWrite(ledPwmChannel(id, 2), (uint8_t)((blue * led_power[id]) / 100));
}

static uint8_t ledPwmChannel(uint8_t id, uint8_t color_index)
{
    return (uint8_t)((id * LED_PWM_CH_PER_LED) + color_index);
}

static bool ledParseCommand(const char *cmd, uint8_t *id, LED_COLOR *color)
{
    char first[8];
    char second[8];
    char third[8];
    const char *cursor = cmd;

    if (!ledReadToken(&cursor, first, sizeof(first))) {
        return false;
    }

    if (!ledStringEqual(first, "led")) {
        return false;
    }

    if (!ledReadToken(&cursor, second, sizeof(second))) {
        return false;
    }

    if (second[0] < '1' || second[0] > '3' || second[1] != '\0') {
        return false;
    }

    *id = (uint8_t)(second[0] - '1');

    if (!ledReadToken(&cursor, third, sizeof(third))) {
        return false;
    }

    while (*cursor == ' ') {
        cursor++;
    }

    if (*cursor != '\0') {
        return false;
    }

    return ledColorFromString(third, color);
}

static bool ledReadToken(const char **cmd, char *token, uint8_t token_size)
{
    uint8_t index = 0;

    while (**cmd == ' ') {
        (*cmd)++;
    }

    if (**cmd == '\0') {
        return false;
    }

    while (**cmd != '\0' && **cmd != ' ') {
        if (index >= (token_size - 1)) {
            return false;
        }

        token[index++] = **cmd;
        (*cmd)++;
    }

    token[index] = '\0';
    return true;
}

static bool ledStringEqual(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        char ca = *a;
        char cb = *b;

//         if (ca >= 'A' && ca <= 'Z') {
//             ca = (char)(ca - 'A' + 'a');
//         }

//         if (cb >= 'A' && cb <= 'Z') {
//             cb = (char)(cb - 'A' + 'a');
//         }

        if (ca != cb) {
            return false;
        }

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0');
}

static bool ledColorFromString(const char *str, LED_COLOR *color)
{
    if (ledStringEqual(str, "off")) {
        *color = LED_OFF;
    } else if (ledStringEqual(str, "yellow")) {
        *color = LED_YELLOW;
    } else if (ledStringEqual(str, "white")) {
        *color = LED_WHITE;
    } else if (ledStringEqual(str, "warm")) {
        *color = LED_WARM_WHITE;
    } else {
        return false;
    }

    return true;
}
