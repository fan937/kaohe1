#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f1xx_hal.h"

void motor_set(int16_t pwm);
void motor_stop(void);

#endif /* __MOTOR_H */