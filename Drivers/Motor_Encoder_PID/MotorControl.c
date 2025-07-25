#include "MotorControl.h"

// 全局控制器实例
MotorControl_t motor_control;

/**
 * @brief 电机控制系统初始化
 */
void MotorControl_Init(void)
{
    // 初始化各个模块
    Motor_Init();
    Encoder_Init();
    
    // 初始化双电机PID控制器
    PID_Init(PID_MOTOR_A, 2.6f, 1.3f, 0.0f);
    PID_Init(PID_MOTOR_B, 2.6f, 1.3f, 0.0f);
    
    // 初始化控制系统状态
    for (int i = 0; i < 2; i++) {
        motor_control.motor[i].target_speed = 0.0f;
        motor_control.motor[i].current_speed = 0.0f;
        motor_control.motor[i].pid_enabled = 0;  // 默认关闭PID控制
    }
    
    motor_control.system_enabled = 1;  // 默认使能系统
}

/**
 * @brief 设置指定电机PID参数
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 */
void MotorControl_SetPIDParams(uint8_t motor_id, float kp, float ki, float kd)
{
    if (motor_id < 2) {
        PID_SetParams(motor_id, kp, ki, kd);
    }
}

/**
 * @brief 设置所有电机PID参数
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 */
void MotorControl_SetPIDParams_All(float kp, float ki, float kd)
{
    PID_SetParams(PID_MOTOR_A, kp, ki, kd);
    PID_SetParams(PID_MOTOR_B, kp, ki, kd);
}

/**
 * @brief 设置指定电机目标速度
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @param speed 目标速度（脉冲每秒）
 */
void MotorControl_SetTargetSpeed(uint8_t motor_id, float speed)
{
    if (motor_id < 2) {
        motor_control.motor[motor_id].target_speed = speed;
    }
}

/**
 * @brief 设置双电机目标速度
 * @param speed_left 左电机目标速度（脉冲每秒）
 * @param speed_right 右电机目标速度（脉冲每秒）
 */
void MotorControl_SetTargetSpeed_Dual(float speed_left, float speed_right)
{
    motor_control.motor[MOTOR_CONTROL_LEFT].target_speed = speed_left;
    motor_control.motor[MOTOR_CONTROL_RIGHT].target_speed = speed_right;
}

/**
 * @brief 使能指定电机PID控制
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 */
void MotorControl_EnablePID(uint8_t motor_id)
{
    if (motor_id < 2) {
        motor_control.motor[motor_id].pid_enabled = 1;
        PID_Reset(motor_id);  // 重置PID状态
    }
}

/**
 * @brief 使能所有电机PID控制
 */
void MotorControl_EnablePID_All(void)
{
    motor_control.motor[MOTOR_CONTROL_LEFT].pid_enabled = 1;
    motor_control.motor[MOTOR_CONTROL_RIGHT].pid_enabled = 1;
    PID_Reset(PID_MOTOR_A);
    PID_Reset(PID_MOTOR_B);
}

/**
 * @brief 禁用指定电机PID控制
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 */
void MotorControl_DisablePID(uint8_t motor_id)
{
    if (motor_id < 2) {
        motor_control.motor[motor_id].pid_enabled = 0;
        Motor_Stop(motor_id);  // 停止电机
    }
}

/**
 * @brief 禁用所有电机PID控制
 */
void MotorControl_DisablePID_All(void)
{
    motor_control.motor[MOTOR_CONTROL_LEFT].pid_enabled = 0;
    motor_control.motor[MOTOR_CONTROL_RIGHT].pid_enabled = 0;
    Motor_Stop(MOTOR_ALL);  // 停止所有电机
}

/**
 * @brief 使能控制系统
 */
void MotorControl_EnableSystem(void)
{
    motor_control.system_enabled = 1;
}

/**
 * @brief 禁用控制系统
 */
void MotorControl_DisableSystem(void)
{
    motor_control.system_enabled = 0;
    MotorControl_DisablePID_All();  // 禁用所有PID并停止电机
}

/**
 * @brief 系统更新函数（在定时器中断中调用）
 * 这个函数应该在编码器速度计算完成后调用
 */
void MotorControl_Update(void)
{
    if (!motor_control.system_enabled) return;
    
    // 更新双电机控制
    for (int i = 0; i < 2; i++) {
        // 获取当前速度
        motor_control.motor[i].current_speed = (float)Encoder_GetSpeed_PPS(i);
        
        // 如果PID控制使能，则进行PID计算
        if (motor_control.motor[i].pid_enabled) {
            // 使用新的PID接口
            float pid_output = PID_Calculate(i, motor_control.motor[i].target_speed, 
                                           motor_control.motor[i].current_speed);
            
            // 将PID输出转换为电机控制信号(-100到100转换为-1.0到1.0)
            float motor_command = pid_output / 100.0f;
            
            // 控制电机
            Motor_Run(i, motor_command);
        }
    }
}

/**
 * @brief 手动控制指定电机（关闭PID）
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @param speed 电机速度 (-1.0到1.0)
 */
void MotorControl_Manual(uint8_t motor_id, float speed)
{
    if (motor_id >= 2 || !motor_control.system_enabled) return;
    
    // 关闭指定电机的PID控制
    motor_control.motor[motor_id].pid_enabled = 0;
    
    // 直接控制电机
    Motor_Run(motor_id, speed);
}

/**
 * @brief 手动控制双电机（关闭PID）
 * @param speed_left 左电机速度 (-1.0到1.0)
 * @param speed_right 右电机速度 (-1.0到1.0)
 */
void MotorControl_Manual_Dual(float speed_left, float speed_right)
{
    if (!motor_control.system_enabled) return;
    
    // 关闭所有PID控制
    MotorControl_DisablePID_All();
    
    // 直接控制双电机
    Motor_Run(MOTOR_CONTROL_LEFT, speed_left);
    Motor_Run(MOTOR_CONTROL_RIGHT, speed_right);
}

/**
 * @brief 停止指定电机
 * @param motor_id 电机ID (0=左电机, 1=右电机, 2=所有电机)
 */
void MotorControl_Stop(uint8_t motor_id)
{
    if (motor_id == MOTOR_CONTROL_ALL) {
        MotorControl_Stop_All();
    } else if (motor_id < 2) {
        motor_control.motor[motor_id].pid_enabled = 0;
        Motor_Stop(motor_id);
    }
}

/**
 * @brief 停止所有电机
 */
void MotorControl_Stop_All(void)
{
    MotorControl_DisablePID_All();
    Motor_Stop(MOTOR_ALL);
}

/**
 * @brief 获取指定电机当前速度
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @return 当前速度（脉冲每秒）
 */
float MotorControl_GetCurrentSpeed(uint8_t motor_id)
{
    if (motor_id >= 2) return 0.0f;
    return motor_control.motor[motor_id].current_speed;
}

/**
 * @brief 获取指定电机目标速度
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @return 目标速度（脉冲每秒）
 */
float MotorControl_GetTargetSpeed(uint8_t motor_id)
{
    if (motor_id >= 2) return 0.0f;
    return motor_control.motor[motor_id].target_speed;
}

/**
 * @brief 获取指定电机编码器计数
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @return 编码器计数值
 */
int32_t MotorControl_GetEncoderCount(uint8_t motor_id)
{
    if (motor_id >= 2) return 0;
    return Encoder_GetCount(motor_id);
}

/**
 * @brief 检查指定电机PID是否使能
 * @param motor_id 电机ID (0=左电机, 1=右电机)
 * @return 1=使能, 0=禁用
 */
uint8_t MotorControl_IsPIDEnabled(uint8_t motor_id)
{
    if (motor_id >= 2) return 0;
    return motor_control.motor[motor_id].pid_enabled;
}

/**
 * @brief 检查系统是否使能
 * @return 1=使能, 0=禁用
 */
uint8_t MotorControl_IsSystemEnabled(void)
{
    return motor_control.system_enabled;
}
