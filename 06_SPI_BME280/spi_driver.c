#include "spi_driver.h"

// SPI 传输函数：只做数据传输，不控制 CS
uint8_t SPI_TransmitReceive(uint8_t tx_data)
{
    uint8_t rx_data = 0;

    // 发送并接收数据
    HAL_SPI_TransmitReceive(&hspi1, &tx_data, &rx_data, 1, HAL_MAX_DELAY);


    return rx_data;
}
