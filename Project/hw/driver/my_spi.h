#ifndef MY_SPI_H
#define MY_SPI_H

#include "hw_def.h"

bool spiInit(void);
uint8_t spiTransferByte(uint8_t data);
bool spiWriteRead(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len);

#endif