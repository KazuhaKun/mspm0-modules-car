/*
 * motor_control.c
 *
 *  电机控制实现
 */

#include "motor_control.h"
#include "Encoder.h"

// PID参数定义
#define LINE_PID_KP         0.2f
#define LINE_PID_KI         0.0f
#define LINE_PID_KD         0.08f
#define LINE_PID_INT_LIMIT  100.0f
#define LINE_PID_OUT_LIMIT  50.0f

#define YAW_PID_KP          1.0f
#define YAW_PID_KI          0.0f
#define YAW_PID_KD          0.2f
#define YAW_PID_INT_LIMIT   200.0f
#define YAW_PID_OUT_LIMIT   100.0f

#define SPEED_PID_KP        0.8f
#define SPEED_PID_KI        0.1f
#define SPEED_PID_KD        0.05f
#define SPEED_PID_INT_LIMIT 500.0f
#define SPEED_PID_OUT_LIMIT 100.0f

Motor_Control_t g_motorControl;

/**
 * @brief 初始化电机控制器
 */
void MotorControl_Init(void)
{
    // 初始化电机和编码器
    Motor_Init();
    Encoder_Init();

    // 初始化PID控制器
    PID_Init(&g_motorControl.line_pid, LINE_PID_KP, LINE_PID_KI, LINE_PID_KD, LINE_PID_INT_LIMIT, LINE_PID_OUT_LIMIT);
    PID_Init(&g_motorControl.yaw_pid, YAW_PID_KP, YAW_PID_KI, YAW_PID_KD, YAW_PID_INT_LIMIT, YAW_PID_OUT_LIMIT);
    PID_Init(&g_motorControl.speed_pid_L, SPEED_PID_KP, SPEED_PID_KI, SPEED_PID_KD, SPEED_PID_INT_LIMIT, SPEED_PID_OUT_LIMIT);
    PID_Init(&g_motorControl.speed_pid_R, SPEED_PID_KP, SPEED_PID_KI, SPEED_PID_KD, SPEED_PID_INT_LIMIT, SPEED_PID_OUT_LIMIT);

    // 设置初始状态
    g_motorControl.mode = MOTOR_MODE_STOP;
    g_motorControl.base_speed = 50.0f; // 默认基础速度
    g_motorControl.target_yaw = 0.0f;
}

/**
 * @brief 设置电机控制模式
 * @param mode 新的控制模式
 */
void MotorControl_SetMode(Motor_Mode_t mode)
{
    g_motorControl.mode = mode;
    if (mode == MOTOR_MODE_STOP) {
        MotorControl_Stop();
    }
}

/**
 * @brief 设置基础速度
 * @param speed 基础速度值
 */
void MotorControl_SetBaseSpeed(float speed)
{
    g_motorControl.base_speed = speed;
}

/**
 * @brief 设置目标Yaw角
 * @param yaw 目标Yaw角
 */
void MotorControl_SetTargetYaw(float yaw)
{
    g_motorControl.target_yaw = yaw;
    PID_SetTarget(&g_motorControl.yaw_pid, yaw);
}

/**
 * @brief 更新电机控制状态，应在主循环中定期调用
 */
void MotorControl_Update(void)
{
    float line_correction = 0;
    float yaw_correction = 0;
    float left_speed_target = 0;
    float right_speed_target = 0;

    switch (g_motorControl.mode) {
        case MOTOR_MODE_LINE_FOLLOWING:
            // 循迹模式
            LineTracker_ReadSensors();
            PID_SetTarget(&g_motorControl.line_pid, 0); // 目标是线位置为0
            line_correction = PID_Calculate(&g_motorControl.line_pid, g_lineTracker.linePosition);

            left_speed_target = g_motorControl.base_speed - line_correction;
            right_speed_target = g_motorControl.base_speed + line_correction;
            break;

        case MOTOR_MODE_YAW_CORRECTION:
            // Yaw角闭环模式
            // 获取当前MPU6050的yaw值 (这里假设有一个全局变量或函数获取当前yaw)
            // 暂时使用一个临时值，实际应该从MPU6050获取
            float current_yaw = 0.0f; // TODO: 替换为实际的yaw获取函数
            yaw_correction = PID_Calculate(&g_motorControl.yaw_pid, current_yaw);

            left_speed_target = g_motorControl.base_speed - yaw_correction;
            right_speed_target = g_motorControl.base_speed + yaw_correction;
            break;

        case MOTOR_MODE_MANUAL:
            // 手动模式下，速度由外部直接设置，这里不处理
            return;

        case MOTOR_MODE_STOP:
        default:
            MotorControl_Stop();
            return;
    }

    // 速度闭环
    int16_t current_speed_L = Encoder_Get_Speed_L();
    int16_t current_speed_R = Encoder_Get_Speed_R();

    PID_SetTarget(&g_motorControl.speed_pid_L, left_speed_target);
    float pwm_L = PID_Calculate(&g_motorControl.speed_pid_L, current_speed_L);

    PID_SetTarget(&g_motorControl.speed_pid_R, right_speed_target);
    float pwm_R = PID_Calculate(&g_motorControl.speed_pid_R, current_speed_R);

    // 设置电机PWM
    Motor_Set_Pwm(pwm_L, pwm_R);
}

/**
 * @brief 停止电机
 */
void MotorControl_Stop(void)
{
    Motor_Stop(MOTOR_ALL);
    PID_Reset(&g_motorControl.line_pid);
    PID_Reset(&g_motorControl.yaw_pid);
    PID_Reset(&g_motorControl.speed_pid_L);
    PID_Reset(&g_motorControl.speed_pid_R);
}
