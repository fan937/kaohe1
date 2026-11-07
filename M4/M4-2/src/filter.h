#ifndef __FILTER_H
#define __FILTER_H

#include "stm32f1xx_hal.h"

typedef struct
{
    float pitch;
    float roll;
    float tau;
    float alpha;
    float gyro_only_pitch;
    float gyro_only_roll;
} ComplementaryFilter_t;

void Filter_Init(ComplementaryFilter_t *f, float tau);

/**
 * @brief 互补滤波更新
 * @param ax,ay,az   加速度，单位 g
 * @param gx,gy      角速度，单位 °/s
 * @param dt         采样间隔，单位 s
 */
void Filter_Update(ComplementaryFilter_t *f,
                   float ax, float ay, float az,
                   float gx, float gy, float dt);

#endif