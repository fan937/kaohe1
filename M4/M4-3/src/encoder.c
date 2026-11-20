#include "encoder.h"

#define FILTER_LEN 5
float rpm_buf[FILTER_LEN];
uint8_t filter_idx = 0;

float rpm_filter(float new_rpm)
{
    rpm_buf[filter_idx] = new_rpm;
    filter_idx++;
    if(filter_idx >= FILTER_LEN) filter_idx = 0;
    float sum = 0;
    for(uint8_t i=0; i<FILTER_LEN; i++)
    {
        sum += rpm_buf[i];
    }
    return sum / FILTER_LEN;
}

void Error_Handler(void);
static int32_t encoder_total = 0;
static int32_t encoder_last = 0;

void Encoder_Init(TIM_HandleTypeDef *htim)
{
    HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
    encoder_last = (int32_t)__HAL_TIM_GET_COUNTER(htim);
}

int32_t Encoder_Get_Delta(TIM_HandleTypeDef *htim)
{
    int32_t now = (int32_t)__HAL_TIM_GET_COUNTER(htim);
    int32_t diff = (int32_t)(now - encoder_last);
    if(diff > 32767) diff -= 65536;
    else if(diff < -32768) diff += 65536;
    encoder_total += diff;
    encoder_last = now;
    return diff;
}

int32_t Encoder_Get(TIM_HandleTypeDef *htim)
{
    return encoder_total;
}

void Encoder_Reset(TIM_HandleTypeDef *htim)
{
    encoder_total = 0;
    encoder_last = (int32_t)__HAL_TIM_GET_COUNTER(htim);
    __HAL_TIM_SET_COUNTER(htim, 0);
}