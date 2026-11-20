#include "motor.h"
#include "main.h"

#define PWM_MAX 99

void motor_set(int16_t pwm)
{
    if(pwm > PWM_MAX)
        pwm = PWM_MAX;
    if(pwm < -PWM_MAX)
        pwm = -PWM_MAX;

    if(pwm > 0)
    {
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pwm);
    }
    else if(pwm < 0)
    {
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, -pwm);
    }
    else
    {
        
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    }
}

void motor_stop(void)
{
    motor_set(0);
}