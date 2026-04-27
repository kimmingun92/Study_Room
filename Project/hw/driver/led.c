#include "led.h"
#include <stdint.h>

#define LED_COUNT             1
#define LED_UART              huart3
#define LED_UART_INSTANCE     USART3
#define LED_UART_BAUDRATE     9600
#define LED_RX_BUFFER_SIZE    32

typedef struct {
    GPIO_TypeDef *rg_port;
    uint16_t rg_pin;
    GPIO_TypeDef *b_port;
    uint16_t b_pin;
    LED_COLOR color;
    uint8_t brightness;
} led_tbl_t;

static led_tbl_t led_tbl[LED_COUNT] = {
    {GPIOF, GPIO_PIN_14, GPIOF, GPIO_PIN_15, LED_OFF, 0},
};

static uint8_t led_rx_data;
static char led_rx_buffer[LED_RX_BUFFER_SIZE];
static uint8_t led_rx_index;

static void ledWrite(uint8_t id, GPIO_PinState rg_state, GPIO_PinState b_state);
static void ledSetCommandColor(uint8_t id, const char *color);
static void ledUartStartReceive(void);
static void ledUartSendString(const char *str);
static int ledStringEqual(const char *a, const char *b);

void ledInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    LED_UART.Init.BaudRate = LED_UART_BAUDRATE;
    HAL_UART_Init(&LED_UART);
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    setLedColor(0, LED_OFF);
    ledUartStartReceive();
    ledUartSendString("LED UART ready\r\n");
}

void ledCommandProcess(const char *cmd)
{
    uint8_t id = 0;
    const char *color = cmd;

    if (cmd == NULL) {
        return;
    }

    while (*color == ' ') {
        color++;
    }

    if ((color[0] == 'l' || color[0] == 'L') &&
        (color[1] == 'e' || color[1] == 'E') &&
        (color[2] == 'd' || color[2] == 'D') &&
        color[3] >= '1' && color[3] <= '9') {
        id = (uint8_t)(color[3] - '1');
        color += 4;

        while (*color == ' ') {
            color++;
        }
    }

    ledSetCommandColor(id, color);
}

void setLedColor(uint8_t id, LED_COLOR color)
{
    if (id >= LED_COUNT) {
        return;
    }

    led_tbl[id].color = color;

    switch (color) {
        case LED_OFF:
            ledWrite(id, GPIO_PIN_RESET, GPIO_PIN_RESET);
            break;

        case LED_YELLOW:
            ledWrite(id, GPIO_PIN_SET, GPIO_PIN_RESET);
            break;

        case LED_WHITE:
        case LED_WARM_WHITE:
            ledWrite(id, GPIO_PIN_SET, GPIO_PIN_SET);
            break;

        case LED_RED:
        case LED_GREEN:
            ledWrite(id, GPIO_PIN_SET, GPIO_PIN_RESET);
            break;

        case LED_BLUE:
            ledWrite(id, GPIO_PIN_RESET, GPIO_PIN_SET);
            break;

        default:
            ledWrite(id, GPIO_PIN_RESET, GPIO_PIN_RESET);
            break;
    }
}

LED_COLOR getLedColor(uint8_t id)
{
    if (id >= LED_COUNT) {
        return LED_OFF;
    }

    return led_tbl[id].color;
}

void setLedPower(uint8_t id, uint8_t brightness)
{
    if (id >= LED_COUNT) {
        return;
    }

    led_tbl[id].brightness = brightness;

    if (brightness == 0) {
        setLedColor(id, LED_OFF);
    }
}

uint8_t getLedPower(uint8_t id)
{
    if (id >= LED_COUNT) {
        return 0;
    }

    return led_tbl[id].brightness;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != LED_UART_INSTANCE) {
        return;
    }

    if (led_rx_data == '\r' || led_rx_data == '\n') {
        if (led_rx_index > 0) {
            led_rx_buffer[led_rx_index] = '\0';
            ledCommandProcess(led_rx_buffer);
            led_rx_index = 0;
        }
    } else if (led_rx_index < (LED_RX_BUFFER_SIZE - 1)) {
        led_rx_buffer[led_rx_index++] = (char)led_rx_data;
    } else {
        led_rx_index = 0;
    }

    ledUartStartReceive();
}

static void ledWrite(uint8_t id, GPIO_PinState rg_state, GPIO_PinState b_state)
{
    HAL_GPIO_WritePin(led_tbl[id].rg_port, led_tbl[id].rg_pin, rg_state);
    HAL_GPIO_WritePin(led_tbl[id].b_port, led_tbl[id].b_pin, b_state);
}

static void ledSetCommandColor(uint8_t id, const char *color)
{
    if (ledStringEqual(color, "off")) {
        setLedColor(id, LED_OFF);
        ledUartSendString("LED off\r\n");
    } else if (ledStringEqual(color, "yellow")) {
        setLedColor(id, LED_YELLOW);
        ledUartSendString("LED yellow\r\n");
    } else if (ledStringEqual(color, "white")) {
        setLedColor(id, LED_WHITE);
        ledUartSendString("LED white\r\n");
    } else if (ledStringEqual(color, "warm")) {
        setLedColor(id, LED_WARM_WHITE);
        ledUartSendString("LED warm\r\n");
    } else {
        ledUartSendString("Unknown command\r\n");
    }
}

static void ledUartStartReceive(void)
{
    HAL_UART_Receive_IT(&LED_UART, &led_rx_data, 1);
}

static void ledUartSendString(const char *str)
{
    HAL_UART_Transmit(&LED_UART, (uint8_t *)str, (uint16_t)strlen(str), 100);
}

static int ledStringEqual(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        char ca = *a;
        char cb = *b;

        if (ca >= 'A' && ca <= 'Z') {
            ca = (char)(ca - 'A' + 'a');
        }

        if (cb >= 'A' && cb <= 'Z') {
            cb = (char)(cb - 'A' + 'a');
        }

        if (ca != cb) {
            return 0;
        }

        a++;
        b++;
    }

    while (*a == ' ') {
        a++;
    }

    return (*a == '\0' && *b == '\0');
}

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&LED_UART);
}
