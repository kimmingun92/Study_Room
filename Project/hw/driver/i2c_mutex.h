#ifndef I2C_MUTEX_H
#define I2C_MUTEX_H

#include <hw_def.h>
#include <def.h>

/* hi2c1 공유 버스 보호용 Mutex */
void    i2cMutexInit(void);
bool    i2cMutexTake(uint32_t timeout_ms);
void    i2cMutexGive(void);

#endif
