#include "pid.h"
#include "math.h"

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float out_max, float intmax)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->out_max = out_max;
    pid->integral_max = intmax;
    pid->set_rpm = 0.0f;
    pid->actual_rpm = 0.0f;
    pid->error = 0.0f;
    pid->error_last = 0.0f;
    pid->integral = 0.0f;
    pid->out = 0.0f;
    pid->diff_last = 0.0f;
    pid->alpha = 0.2f; // 微分低通滤波系数
}

float PID_Calc(PID_TypeDef *pid, float actual, float dt)
{
    if(dt <= 0.0f) return 0.0f; 

    pid->actual_rpm = actual;
    pid->error = pid->set_rpm - pid->actual_rpm;

    // ================== 1. 比例项 P ==================
    float P = pid->Kp * pid->error;

    // ================== 2. 微分项 D (带滤波) ==================
    static float last_actual = 0.0f; 
    float diff_val = -(pid->actual_rpm - last_actual) / dt; 
    last_actual = pid->actual_rpm;

    float D = pid->Kd * (pid->diff_last * (1.0f - pid->alpha) + diff_val * pid->alpha);
    pid->diff_last = D; 

    // ================== 3. 积分项 I (抗饱和 + 限幅) ==================
    float out_without_I = P + D;
    float I = pid->Ki * pid->integral;
    
    float sep_threshold = 15.0f; 
    
    // 抗积分饱和 & 积分分离
    if (pid->error>0.0f||fabs(pid->error) < sep_threshold){ 
        if ((out_without_I + I < pid->out_max && out_without_I + I > -pid->out_max) || 
            (out_without_I + I >= pid->out_max && pid->error < 0) || 
            (out_without_I + I <= -pid->out_max && pid->error > 0))
        {
            pid->integral += pid->error * dt;
        }
    }

    // 积分限幅 
    if(pid->integral > pid->integral_max)  pid->integral = pid->integral_max;
    if(pid->integral < -pid->integral_max) pid->integral = -pid->integral_max;

    I = pid->Ki * pid->integral;

    // ================== 4. 计算总输出与限幅 ==================
    pid->out = P + I + D;

    if(pid->out > pid->out_max)  pid->out = pid->out_max;
    if(pid->out < -pid->out_max) pid->out = -pid->out_max;

    pid->error_last = pid->error;
    return pid->out;
}

void PID_Reset(PID_TypeDef *pid)
{
    pid->error = 0.0f;
    pid->error_last = 0.0f;
    pid->diff_last = 0.0f;
    pid->integral = 0.0f;
    pid->out = 0.0f;
}