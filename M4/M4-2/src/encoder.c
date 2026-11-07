#include "encoder.h"

void Error_Handler(void);
static int32_t encoder_total = 0;
static int32_t encoder_last = 0;

void Encoder_Init(TIM_HandleTypeDef *htim)
{
    htim->Instance = TIM3;
    htim->Init.Prescaler = 0;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 65535;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    TIM_Encoder_InitTypeDef sEnc = {0};
    sEnc.EncoderMode = TIM_ENCODERMODE_TI12;

    sEnc.IC1Filter = 0x0F;
    sEnc.IC1Polarity = TIM_ICPOLARITY_BOTHEDGE;
    sEnc.IC1Prescaler = TIM_ICPSC_DIV1;
    sEnc.IC1Selection = TIM_ICSELECTION_DIRECTTI;

    sEnc.IC2Filter = 0x0F;
    sEnc.IC2Polarity = TIM_ICPOLARITY_BOTHEDGE;
    sEnc.IC2Prescaler = TIM_ICPSC_DIV1;
    sEnc.IC2Selection = TIM_ICSELECTION_DIRECTTI;

    if (HAL_TIM_Encoder_Init(htim, &sEnc) != HAL_OK)
    {
        Error_Handler();
    }
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