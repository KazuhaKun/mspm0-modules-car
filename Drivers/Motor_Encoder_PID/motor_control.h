/*
 * motor_control.h
 *
 *  电机控制头文件
 */

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#include "pid.h"
#include "linetracker.h"
#include "mpu6050.h"
#include "Motor.h"

// 电机控制模式
typedef enum {
    MOTOR_MODE_STOP,            // 停止
    MOTOR_MODE_LINE_FOLLOWING,  // 循迹
    MOTOR_MODE_YAW_CORRECTION,  // Yaw角闭环
    MOTOR_MODE_MANUAL           // 手动控制
} Motor_Mode_t;

// 电机控制结构体
typedef struct {
    PID_Controller_t line_pid;      // 循迹PID
    PID_Controller_t yaw_pid;       // Yaw角PID
    PID_Controller_t speed_pid_L;   // 左轮速度PID
    PID_Controller_t speed_pid_R;   // 右轮速度PID

    Motor_Mode_t mode;              // 当前模式

    float base_speed;               // 基础速度
    float target_yaw;               // 目标Yaw角

} Motor_Control_t;

// 全局变量声明
extern Motor_Control_t g_motorControl;

void MotorControl_Init(void);
void MotorControl_SetMode(Motor_Mode_t mode);
void MotorControl_SetBaseSpeed(float speed);
void MotorControl_SetTargetYaw(float yaw);
void MotorControl_Update(void);
void MotorControl_Stop(void);

#endif /* MOTOR_CONTROL_H_ */
