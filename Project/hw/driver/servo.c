#include "servo.h"

extern TIM_HandleTypeDef htim3;

#define SERVO_COUNT 4

#define SERVO_CLOSE_US  600
#define SERVO_OPEN_US   1200

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    bool doorState;
} Servo_t;

static Servo_t servo_tbl[SERVO_COUNT] =
{
    { &htim3, TIM_CHANNEL_1, false }, // 0번 방 문
    { &htim3, TIM_CHANNEL_2, false }, // 1번 방 문
    { &htim3, TIM_CHANNEL_3, false }, // 2번 방 문
    { &htim3, TIM_CHANNEL_4, false }  // 전체 문
};

void servoInit(void)
{
    pwmStart();

    for(uint8_t i = 0; i < SERVO_COUNT; i++)
    {
        changeDoorState(i, DOOR_CLOSE);
    }
}

void changeDoorState(uint8_t id, bool doorState)
{
    if(id >= SERVO_COUNT)
        return;

    servo_tbl[id].doorState = doorState;

    if(doorState == DOOR_OPEN)
    {
        pwmWriteUs(servo_tbl[id].htim,
                   servo_tbl[id].channel,
                   SERVO_OPEN_US);
    }
    else
    {
        pwmWriteUs(servo_tbl[id].htim,
                   servo_tbl[id].channel,
                   SERVO_CLOSE_US);
    }
}