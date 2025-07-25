/*
 * pid.c
 *
 *  通用PID控制器实现
 */

#include "pid.h"

/**
 * @brief 初始化PID控制器
 * @param pid 指向PID控制器结构体的指针
 * @param Kp 比例增益
 * @param Ki 积分增益
 * @param Kd 微分增益
 * @param integral_limit 积分限幅
 * @param output_limit 输出限幅
 */
void PID_Init(PID_Controller_t *pid, float Kp, float Ki, float Kd, float integral_limit, float output_limit)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

/**
 * @brief 设置PID目标值
 * @param pid 指向PID控制器结构体的指针
 * @param target 目标值
 */
void PID_SetTarget(PID_Controller_t *pid, float target)
{
    pid->target = target;
}

/**
 * @brief 计算PID输出
 * @param pid 指向PID控制器结构体的指针
 * @param actual 实际值
 * @return PID输出值
 */
float PID_Calculate(PID_Controller_t *pid, float actual)
{
    pid->actual = actual;
    pid->error = pid->target - pid->actual;

    // 积分项
    pid->integral += pid->error;
    // 积分限幅
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }

    // PID计算
    pid->output = pid->Kp * pid->error +
                  pid->Ki * pid->integral +
                  pid->Kd * (pid->error - pid->last_error);

    // // 输出限幅
    // if (pid->output > pid->output_limit) {
    //     pid->output = pid->output_limit;
    // } else if (pid->output < -pid->output_limit) {
    //     pid->output = -pid->output_limit;
    // }


// ==================== 新增逻辑：抗积分饱和 (Anti-Windup) ====================
    float output_real = pid->output; // 保存未经限制的原始输出

    // 1. 对输出进行限幅
    if (pid->output > pid->output_limit) {
        pid->output = pid->output_limit;
    } else if (pid->output < -pid->output_limit) {
        pid->output = -pid->output_limit;
    }

    // 2. 计算输出偏差 (原始输出与限幅后输出的差值)
    float output_error = output_real - pid->output;

    // 3. 如果发生了限幅 (output_error不为0)，则反向衰减积分项
    //    这里我们用一个简单的比例方法来衰减，防止积分持续累加
    if (output_error != 0.0f && pid->Ki != 0.0f) {
        pid->integral -= (output_error / pid->Ki);
    }
// ========================================================================

    pid->last_error = pid->error;

    return pid->output;
}

/**
 * @brief 重置PID控制器状态
 * @param pid 指向PID控制器结构体的指针
 */
void PID_Reset(PID_Controller_t *pid)
{
    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}
