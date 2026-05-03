#ifndef MY_RFID_H
#define MY_RFID_H

#include "hw_def.h"

#define RFID_UID_MAX_LEN 10

typedef enum
{
    RFID_OK      = 0,
    RFID_NOTAG   = 1,
    RFID_ERROR   = 2,
    RFID_TIMEOUT = 3
} rfid_status_t;

typedef struct
{
    uint8_t bytes[RFID_UID_MAX_LEN];
    uint8_t size;
} rfid_uid_t;

bool rfidInit(void);
uint8_t rfidGetVersion(void);
rfid_status_t rfidReadUid(rfid_uid_t *uid);

void rfidProcess(void);

void rfidSetMonitor(bool state);
bool rfidGetMonitor(void);

#endif