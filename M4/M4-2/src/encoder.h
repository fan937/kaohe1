#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f1xx_hal.h"


void Encoder_Init(TIM_HandleTypeDef *htim);
int32_t Encoder_Get_Delta(TIM_HandleTypeDef *htim);
int32_t Encoder_Get(TIM_HandleTypeDef *htim);
void Encoder_Reset(TIM_HandleTypeDef *htim);

#endif