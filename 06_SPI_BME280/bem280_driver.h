/*
 * bme280_driver.h
 *
 *  Created on: 2026年8月22日
 *      Author: 15332
 */

#ifndef BME280_DRIVER_H
#define BME280_DRIVER_H

#include "main.h"
#include <stdint.h>

// BME280 校准数据结构体
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

} BME280_CalibData;

// 函数声明
uint8_t BME280_ReadChipID(void);
uint8_t BME280_ReadReg(uint8_t reg);
HAL_StatusTypeDef BME280_ReadCalibration(BME280_CalibData *calib);
void BME280_PrintCalibration(BME280_CalibData *calib);

HAL_StatusTypeDef BME280_WriteReg(uint8_t reg, uint8_t data); // 写入一个寄存器（SPI）
HAL_StatusTypeDef BME280_Config(void); // 配置 BME280 工作模式
int32_t BME280_ReadTemperature(void); // 读取温度（返回摄氏度 × 100，方便打印小数）

#endif
