#include "my_spi.h"

extern SPI_HandleTypeDef hspi2;

bool spiInit(void)
{
    return true;
}

uint8_t spiTransferByte(uint8_t data)
{
    uint8_t rx_data = 0;

    HAL_SPI_TransmitReceive(&hspi2, &data, &rx_data, 1, 100);

    return rx_data;
}

bool spiWriteRead(uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len)
{
    if (HAL_SPI_TransmitReceive(&hspi2, tx_buf, rx_buf, len, 100) == HAL_OK)
        return true;

    return false;
}