#ifndef __PID_H
#define __PID_H
#include "stm32f1xx_hal.h"

typedef struct
{
    float Kp;
    float Ki;
    float Kd;
    float set_rpm;
    float actual_rpm;
    float error;
    float error_last;
    float integral;
    float out;
    float out_max;
    float integral_max;
    float diff_last;
    float alpha;
    
} PID_TypeDef;
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float out_max, float intmax);
float PID_Calc(PID_TypeDef *pid, float actual,float dt);
void PID_Reset(PID_TypeDef *pid);
#endif