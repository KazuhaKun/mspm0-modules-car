#ifndef __MOTOR_H
#define __MOTOR_H

#include "ti_msp_dl_config.h"

// 电机控制函数
void Motor_Init(void);                                    // 初始化电机
void Motor_SetDirection(uint8_t motor_id, int8_t direction); // 设置方向: motor_id(0=A, 1=B), direction(1=正转, -1=反转, 0=停止)
void Motor_SetPWM(uint8_t motor_id, float duty);         // 设置PWM占空比 (0.0-1.0)
void Motor_Run(uint8_t motor_id, float speed);           // 综合控制函数 (-1.0到1.0)
void Motor_Stop(uint8_t motor_id);                       // 停止指定电机 (0=A, 1=B, 2=全部)
void Motor_PWM_Start(void);                              // 启动PWM
void Motor_PWM_Stop(void);                               // 停止PWM
uint8_t Motor_IsRunning(void);                           // 检查PWM是否运行

// 宏定义
#define MIN_PWM_DUTY 0.1f                                // 最小PWM占空比
#define MOTOR_A      0                                   // 电机A ID
#define MOTOR_B      1                                   // 电机B ID
#define MOTOR_ALL    2                                   // 全部电机

#endif
