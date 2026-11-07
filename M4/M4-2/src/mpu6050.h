#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f1xx_hal.h"

#define MPU6050_CFG 0x1A
#define MPU6050_ADDR 0x68
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_PWR_MGMT_1 0x6B
#define ACCEL_Config 0x1C
#define GYRO_Config 0x1B
#define DATA_START 0x3B

#define ACCEL_SCALE 16384.0f
#define GYRO_SCALE 131.0f
typedef struct {
    int16_t a_x;
    int16_t a_y;
    int16_t a_z;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6050_Data;
uint8_t MPU6050_Init(I2C_HandleTypeDef *hi2c);
void MPU6050_ReadData(I2C_HandleTypeDef *hi2c, MPU6050_Data *data);
#endif