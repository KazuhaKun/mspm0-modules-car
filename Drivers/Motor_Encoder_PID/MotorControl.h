#ifndef __MOTOR_CONTROL_H
#define __MOTOR_CONTROL_H

#include "Motor.h"
#include "Encoder.h"
#include "PID.h"

// 单个电机控制结构体
typedef struct {
    float target_speed;       // 目标速度
    float current_speed;      // 当前速度
    uint8_t pid_enabled;      // PID控制使能
} MotorControlSingle_t;

// 双电机控制系统结构体
typedef struct {
    MotorControlSingle_t motor[2];  // 双电机控制 [0=左电机A, 1=右电机B]
    uint8_t system_enabled;         // 系统总使能
} MotorControl_t;

// 全局控制器实例
extern MotorControl_t motor_control;

// 系统初始化函数
void MotorControl_Init(void);

// PID控制相关函数
void MotorControl_SetPIDParams(uint8_t motor_id, float kp, float ki, float kd);  // 设置指定电机PID参数
void MotorControl_SetPIDParams_All(float kp, float ki, float kd);                // 设置所有电机PID参数
void MotorControl_SetTargetSpeed(uint8_t motor_id, float speed);                 // 设置目标速度
void MotorControl_SetTargetSpeed_Dual(float speed_left, float speed_right);      // 设置双电机目标速度
void MotorControl_EnablePID(uint8_t motor_id);                                   // 使能指定电机PID
void MotorControl_EnablePID_All(void);                                           // 使能所有电机PID
void MotorControl_DisablePID(uint8_t motor_id);                                  // 禁用指定电机PID
void MotorControl_DisablePID_All(void);                                          // 禁用所有电机PID

// 系统控制函数
void MotorControl_EnableSystem(void);                                            // 使能系统
void MotorControl_DisableSystem(void);                                           // 禁用系统
void MotorControl_Update(void);                                                  // 系统更新函数

// 手动控制函数
void MotorControl_Manual(uint8_t motor_id, float speed);                         // 手动控制指定电机
void MotorControl_Manual_Dual(float speed_left, float speed_right);              // 手动控制双电机
void MotorControl_Stop(uint8_t motor_id);                                        // 停止指定电机
void MotorControl_Stop_All(void);                                                // 停止所有电机

// 状态获取函数
float MotorControl_GetCurrentSpeed(uint8_t motor_id);                            // 获取当前速度
float MotorControl_GetTargetSpeed(uint8_t motor_id);                             // 获取目标速度
int32_t MotorControl_GetEncoderCount(uint8_t motor_id);                          // 获取编码器计数
uint8_t MotorControl_IsPIDEnabled(uint8_t motor_id);                             // 检查PID是否使能
uint8_t MotorControl_IsSystemEnabled(void);                                     // 检查系统是否使能

// 宏定义
#define MOTOR_CONTROL_LEFT     0                                                 // 左电机ID
#define MOTOR_CONTROL_RIGHT    1                                                 // 右电机ID
#define MOTOR_CONTROL_ALL      2                                                 // 所有电机

#endif
