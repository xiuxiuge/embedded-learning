/*
 * spi_driver.h
 *
 *  Created on: 2026年8月21日
 *      Author: 15332
 */

#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "stm32f4xx_hal.h"

// CS 引脚定义（用 PA4）
#define BME280_CS_PIN       GPIO_PIN_4
#define BME280_CS_PORT      GPIOA

// CS 控制宏
#define BME280_CS_LOW()     HAL_GPIO_WritePin(BME280_CS_PORT, BME280_CS_PIN, GPIO_PIN_RESET) // 拉低 CS（选中 BME280）
#define BME280_CS_HIGH()    HAL_GPIO_WritePin(BME280_CS_PORT, BME280_CS_PIN, GPIO_PIN_SET)   // 拉高 CS（释放 BME280）

// SPI 句柄（在 spi_driver.c 中定义）
extern SPI_HandleTypeDef hspi1;

// 函数声明
uint8_t SPI_TransmitReceive(uint8_t tx_data); // 发送一个字节并接收一个字节

#endif
