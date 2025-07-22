#ifndef __PID_H
#define __PID_H

#include <stdint.h>

// PID结构体定义
typedef struct {
    float Kp, Ki, Kd;          // PID参数
    float error;               // 当前误差
    float last_error;          // 上次误差
    float output;              // 输出值
    float max_output;          // 输出限幅最大值
    float min_output;          // 输出限幅最小值
} PID_TypeDef;

// PID控制函数
void PID_Init(uint8_t motor_id, float kp, float ki, float kd);           // 初始化PID参数
void PID_SetParams(uint8_t motor_id, float kp, float ki, float kd);      // 设置PID参数
void PID_SetOutputLimit(uint8_t motor_id, float min_out, float max_out); // 设置输出限幅
float PID_Calculate(uint8_t motor_id, float target, float current);      // PID计算
void PID_Reset(uint8_t motor_id);                                        // 重置PID状态

// 兼容旧接口（保持向后兼容）
int Velocity_A(float Target, float Current);
int Velocity_B(float Target, float Current);
void PID_SetParams_Global(float kp, float ki, float kd);

// 宏定义
#define PID_MOTOR_A      0                    // 电机A ID
#define PID_MOTOR_B      1                    // 电机B ID
#define PID_DEFAULT_MAX  100.0f               // 默认输出最大值
#define PID_DEFAULT_MIN  -100.0f              // 默认输出最小值

#endif
