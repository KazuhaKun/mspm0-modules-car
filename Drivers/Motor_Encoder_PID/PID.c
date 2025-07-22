#include "PID.h"

// PID控制器实例 - 双电机
static PID_TypeDef pid_controllers[2];

// 全局PID参数（保持向后兼容）
float Kp = 2.6f, Ki = 1.3f, Kd = 0;

/**
 * @brief 初始化PID控制器
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param kp 比例系数
 * @param ki 积分系数  
 * @param kd 微分系数
 */
void PID_Init(uint8_t motor_id, float kp, float ki, float kd)
{
    if (motor_id >= 2) return;
    
    pid_controllers[motor_id].Kp = kp;
    pid_controllers[motor_id].Ki = ki;
    pid_controllers[motor_id].Kd = kd;
    pid_controllers[motor_id].error = 0;
    pid_controllers[motor_id].last_error = 0;
    pid_controllers[motor_id].output = 0;
    pid_controllers[motor_id].max_output = PID_DEFAULT_MAX;
    pid_controllers[motor_id].min_output = PID_DEFAULT_MIN;
}

/**
 * @brief 设置PID参数
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 */
void PID_SetParams(uint8_t motor_id, float kp, float ki, float kd)
{
    if (motor_id >= 2) return;
    
    pid_controllers[motor_id].Kp = kp;
    pid_controllers[motor_id].Ki = ki;
    pid_controllers[motor_id].Kd = kd;
}

/**
 * @brief 设置输出限幅
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param min_out 最小输出值
 * @param max_out 最大输出值
 */
void PID_SetOutputLimit(uint8_t motor_id, float min_out, float max_out)
{
    if (motor_id >= 2) return;
    
    pid_controllers[motor_id].min_output = min_out;
    pid_controllers[motor_id].max_output = max_out;
}

/**
 * @brief PID计算（增量式PID）
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param target 目标值
 * @param current 当前值
 * @return PID输出值
 */
float PID_Calculate(uint8_t motor_id, float target, float current)
{
    if (motor_id >= 2) return 0;
    
    PID_TypeDef *pid = &pid_controllers[motor_id];
    
    // 更新误差
    pid->last_error = pid->error;
    pid->error = target - current;
    
    // 增量式PID计算
    pid->output += pid->error * pid->Kp + (pid->error - pid->last_error) * pid->Ki;
    
    // 输出限幅
    if (pid->output > pid->max_output) {
        pid->output = pid->max_output;
    } else if (pid->output < pid->min_output) {
        pid->output = pid->min_output;
    }
    
    return pid->output;
}

/**
 * @brief 重置PID状态
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 */
void PID_Reset(uint8_t motor_id)
{
    if (motor_id >= 2) return;
    
    pid_controllers[motor_id].error = 0;
    pid_controllers[motor_id].last_error = 0;
    pid_controllers[motor_id].output = 0;
}

// ============== 兼容性函数 ==============

/**
 * @brief 设置全局PID参数（保持向后兼容）
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 */
void PID_SetParams_Global(float kp, float ki, float kd)
{
    Kp = kp;
    Ki = ki;
    Kd = kd;
    
    // 同时更新两个电机的参数
    PID_SetParams(PID_MOTOR_A, kp, ki, kd);
    PID_SetParams(PID_MOTOR_B, kp, ki, kd);
}

/**
 * @brief 电机A的PID闭环控制（兼容旧接口）
 * @param Target 目标速度
 * @param Current 当前速度
 * @return 控制输出(-100到100)
 */
int Velocity_A(float Target, float Current)
{
    // 确保电机A的PID已初始化
    static uint8_t initialized_A = 0;
    if (!initialized_A) {
        PID_Init(PID_MOTOR_A, Kp, Ki, Kd);
        initialized_A = 1;
    }
    
    return (int)PID_Calculate(PID_MOTOR_A, Target, Current);
}

/**
 * @brief 电机B的PID闭环控制（兼容旧接口）
 * @param Target 目标速度
 * @param Current 当前速度
 * @return 控制输出(-100到100)
 */
int Velocity_B(float Target, float Current)
{
    // 确保电机B的PID已初始化
    static uint8_t initialized_B = 0;
    if (!initialized_B) {
        PID_Init(PID_MOTOR_B, Kp, Ki, Kd);
        initialized_B = 1;
    }
    
    return (int)PID_Calculate(PID_MOTOR_B, Target, Current);
}
