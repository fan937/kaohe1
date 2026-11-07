#include "main.h"
#include "usart.h"
#include "encoder.h"
#include "mpu6050.h"
#include "filter.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define PPR                 20
#define GEAR_RATIO          1.0f
#define ENCODER_RESOLUTION  (PPR * GEAR_RATIO * 4)
#define ENCODER_PERIOD      1.0f
#define TAU                 1.0f
#define IMU_DT              0.01f
#define CALI_SAMPLE_CNT     100

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;

volatile uint8_t sample_flag = 0;
uint32_t t_start;

ComplementaryFilter_t imu_filter;
float gx_bias = 0.0f;
float gy_bias = 0.0f;
float gz_bias = 0.0f;

int32_t enc_pulse_sum = 0;
float speed_rpm_last = 0.0f;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        sample_flag = 1;
    }
}

uint8_t MPU_CalibrateGyroOffset(I2C_HandleTypeDef *hi2c, uint16_t sample_num)
{
    if(sample_num == 0) return 1;
    int64_t sum_gx=0,sum_gy=0,sum_gz=0;
    MPU6050_Data temp_data;
    for(uint16_t i=0;i<sample_num;i++)
    {
        MPU6050_ReadData(hi2c,&temp_data);
            
        sum_gx += temp_data.gx;
        sum_gy += temp_data.gy;
        sum_gz += temp_data.gz;
        HAL_Delay(10);
    }
    gx_bias = ((float)sum_gx / sample_num)/131.0f;
    gy_bias = ((float)sum_gy / sample_num)/131.0f;
    gz_bias = ((float)sum_gz / sample_num)/131.0f;
    return 0;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Board_Init_USART1();
    MX_TIM2_Init();
    MX_I2C1_Init();

    if (MPU6050_Init(&hi2c1) != 0)
    {
        const char *error_msg = "MPU6050 识别失败\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)error_msg, strlen(error_msg), HAL_MAX_DELAY);
        while (1);
    }

    //上电陀螺校准！上电时面包板必须完全静止不动
    MPU_CalibrateGyroOffset(&hi2c1, CALI_SAMPLE_CNT);

    Encoder_Init(&htim3);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start_IT(&htim2);

    Filter_Init(&imu_filter, TAU);

    t_start = HAL_GetTick();
    //const char *hdr = "t,filter_pitch,filter_roll,gyro_only_pitch,gyro_only_roll\r\n";
    //t,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,gyro_only_pitch,gyro_only_roll,window_pulse,speed_rpm
   // HAL_UART_Transmit(&huart1, (uint8_t *)hdr, strlen(hdr), HAL_MAX_DELAY);

    MPU6050_Data mpu_data;
    char buf[256];

    while (1)
    {
        if (sample_flag)
        {
            sample_flag = 0;
            static uint16_t enc_tick_cnt = 0;
            float t = (HAL_GetTick() - t_start) / 1000.0f;
            MPU6050_ReadData(&hi2c1, &mpu_data);

            //只调用一次！获取10ms脉冲增量
            int32_t delta_pulse = Encoder_Get_Delta(&htim3);

            //IMU换算，减去陀螺零偏
            float ax_g = mpu_data.a_x / 16384.0f;
            float ay_g = mpu_data.a_y / 16384.0f;
            float az_g = mpu_data.a_z / 16384.0f;

            float gx_dps = mpu_data.gx / 131.0f - gx_bias;
            float gy_dps = mpu_data.gy / 131.0f - gy_bias;
            float gz_dps = mpu_data.gz / 131.0f - gz_bias;

            //互补滤波，dt严格0.01s
            Filter_Update(&imu_filter, ax_g, ay_g, az_g, gx_dps, gy_dps, IMU_DT);

            //编码器1s窗口累加
            enc_tick_cnt++;
            enc_pulse_sum += delta_pulse;
            if(enc_tick_cnt >= 100 )
            {
                speed_rpm_last = ((float)enc_pulse_sum / ENCODER_RESOLUTION) * (60.0f / ENCODER_PERIOD);
                enc_tick_cnt = 0;
                enc_pulse_sum = 0;
 /*               int len = snprintf(buf, sizeof(buf),
                               "%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                               t,
                               imu_filter.pitch, imu_filter.roll,
                               imu_filter.gyro_only_pitch, imu_filter.gyro_only_roll
                               );
                               HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
 */           }

           int len = snprintf(buf, sizeof(buf),
                               "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\r\n",
                               t,ax_g, ay_g, az_g,gx_dps, gy_dps, gz_dps,
                               imu_filter.pitch
                               );
//t,ax_g, ay_g, az_g,gx_dps, gy_dps, gz_dps,imu_filter.pitch, imu_filter.roll,enc_pulse_sum,speed_rpm_last
                               
                               HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
        }
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
void __weak HAL_TIMEx_CommutCallback(TIM_HandleTypeDef *htim){UNUSED(htim);}
void __weak HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim){UNUSED(htim);}
void *__wrap__sbrk(ptrdiff_t incr){(void)incr;return NULL;}