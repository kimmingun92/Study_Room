#include "i2c_mutex.h"

static osMutexId_t hI2cMutex = NULL;

void i2cMutexInit(void)
{
    hI2cMutex = osMutexNew(NULL);
}

bool i2cMutexTake(uint32_t timeout_ms)
{
    if (hI2cMutex == NULL) return true;  // 초기화 전엔 통과
    return (osMutexAcquire(hI2cMutex, timeout_ms) == osOK);
}

void i2cMutexGive(void)
{
    if (hI2cMutex == NULL) return;
    osMutexRelease(hI2cMutex);
}
