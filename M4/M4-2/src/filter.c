#define _USE_MATH_DEFINES
#include "filter.h"
#include "math.h"

void Filter_Init(ComplementaryFilter_t *f, float tau)
{
    f->pitch = 0.0f;
    f->roll  = 0.0f;
    f->tau   = tau;
    f->alpha = 0.98f;
    f->gyro_only_pitch = 0.0f;
    f->gyro_only_roll  = 0.0f;
}

void Filter_Update(ComplementaryFilter_t *f,
                   float ax, float ay, float az,
                   float gx, float gy, float dt)
{
    float pitch_acc, roll_acc;

    // 按题目坐标系：X 车头前，Y 车身右，Z 竖直向上
    // pitch：绕 Y；roll：绕 X
    pitch_acc = atan2f(-ay, az) * 180.0f / (float)M_PI;
    roll_acc  = atan2f(-ax,  az) * 180.0f / (float)M_PI;
    f->gyro_only_pitch += gy * dt;
    f->gyro_only_roll  += gx * dt;

    // 更新融合系数
    f->alpha = f->tau / (f->tau + dt);

    // 陀螺积分预测
    f->pitch += gy * dt;
    f->roll  += gx * dt;

    // 互补融合
    f->pitch = f->alpha * f->pitch + (1.0f - f->alpha) * pitch_acc;
    f->roll  = f->alpha * f->roll  + (1.0f - f->alpha) * roll_acc;
}