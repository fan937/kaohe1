#include "mpu6050.h"
#include "main.h"
#include <stdio.h>

static uint8_t Mpu_ReadReg(I2C_HandleTypeDef *hi2c, uint8_t reg)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 50);
    return value;
}

static HAL_StatusTypeDef Mpu_WriteReg(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val)
{
    return HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 50);
}

uint8_t MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t id = Mpu_ReadReg(hi2c, MPU6050_WHO_AM_I);
    printf("WHO_AM_I: %02X\r\n", id);
    if (id != 0x68)
    {
        printf("ID ERROR!\r\n");
        return 1;
    }

    HAL_StatusTypeDef st;

    st = Mpu_WriteReg(hi2c, MPU6050_PWR_MGMT_1, 0x80);
    printf("Write RST(0x80), ret = %d\r\n", st);
    HAL_Delay(100);

    st = Mpu_WriteReg(hi2c, MPU6050_PWR_MGMT_1, 0x00);
    printf("Write WAKE(0x00), ret = %d\r\n", st);
    HAL_Delay(50);

    Mpu_WriteReg(hi2c, 0x6C, 0x00);

    st = Mpu_WriteReg(hi2c, ACCEL_Config, 0x00);
    printf("Write ACCEL, ret = %d\r\n", st);

    st = Mpu_WriteReg(hi2c, GYRO_Config, 0x00);
    printf("Write GYRO, ret = %d\r\n", st);

    st = Mpu_WriteReg(hi2c, MPU6050_CFG, 0x06);
    printf("Write CFG, ret = %d\r\n", st);

    return 0;
}

void MPU6050_ReadData(I2C_HandleTypeDef *hi2c, MPU6050_Data *data)
{
    uint8_t buf_acc[6];
    uint8_t buf_gyro[6];

    HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR << 1, 0x3B, I2C_MEMADD_SIZE_8BIT, buf_acc, 6, 20);
    HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR << 1, 0x43, I2C_MEMADD_SIZE_8BIT, buf_gyro, 6, 20);

    int16_t raw_ax = (buf_acc[0] << 8) | buf_acc[1];
    int16_t raw_ay = (buf_acc[2] << 8) | buf_acc[3];
    int16_t raw_az = (buf_acc[4] << 8) | buf_acc[5];

    int16_t raw_gx = (buf_gyro[0] << 8) | buf_gyro[1];
    int16_t raw_gy = (buf_gyro[2] << 8) | buf_gyro[3];
    int16_t raw_gz = (buf_gyro[4] << 8) | buf_gyro[5];

    data->a_x = raw_ax;
    data->a_y = raw_ay;
    data->a_z = raw_az;
    data->gx  = raw_gx;
    data->gy  = raw_gy;
    data->gz  = raw_gz;
}