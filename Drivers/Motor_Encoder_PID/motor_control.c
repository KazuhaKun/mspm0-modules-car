/*
 * motor_control.c
 *
 *  电机控制实现
 */

#include "motor_control.h"
#include "Encoder.h"
#include "linetracker.h"

// PID参数定义 - 优化循迹PID参数，减小响应速度避免过冲
#define LINE_PID_KP         0.15f   // 减小比例增益，使转向更平滑
#define LINE_PID_KI         0.0f    // 保持积分增益为0，避免积分饱和
#define LINE_PID_KD         0.05f   // 调整微分增益，提高系统响应
#define LINE_PID_INT_LIMIT  50.0f   // 减小积分限幅
#define LINE_PID_OUT_LIMIT  30.0f   // 减小输出限幅，避免速度突变

#define YAW_PID_KP          1.0f
#define YAW_PID_KI          0.0f
#define YAW_PID_KD          0.2f
#define YAW_PID_INT_LIMIT   200.0f
#define YAW_PID_OUT_LIMIT   100.0f

#define SPEED_PID_KP        1.2f    // Increased proportional gain for faster response
#define SPEED_PID_KI        0.2f    // Increased integral gain to eliminate steady-state error
#define SPEED_PID_KD        0.1f    // Increased derivative gain for improved dynamic response
#define SPEED_PID_INT_LIMIT 500.0f
#define SPEED_PID_OUT_LIMIT 100.0f

// 添加速度变化限制，避免速度突变
#define MAX_SPEED_CHANGE 10.0f
// 添加最大速度限制，防止电机跑满
#define MAX_MOTOR_SPEED 30.0f

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
    g_motorControl.base_speed = 30.0f; // 降低默认基础速度，避免过快
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
    // 限制基础速度范围，避免过大
    if (speed > 50.0f) {
        speed = 50.0f;
    } else if (speed < 0.0f) {
        speed = 0.0f;
    }
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
            
            // 特殊处理：当检测到极端情况（只有最边上的传感器有信号）时，限制修正值
            if ((g_lineTracker.sensorBits == 0b0000001) || (g_lineTracker.sensorBits == 0b1000000)) {
                // 只有最边缘的传感器检测到线，限制修正值避免急转
                if (g_lineTracker.sensorBits == 0b0000001) {
                    line_correction = -15.0f; // 限制左转修正
                } else {
                    line_correction = 15.0f;  // 限制右转修正
                }
            } else {
                // 正常情况下使用PID计算
                line_correction = PID_Calculate(&g_motorControl.line_pid, g_lineTracker.linePosition);
            }

            left_speed_target = g_motorControl.base_speed - line_correction;
            right_speed_target = g_motorControl.base_speed + line_correction;
            
            // 限制速度目标值，避免过大
            if (left_speed_target > MAX_MOTOR_SPEED) left_speed_target = MAX_MOTOR_SPEED;
            if (left_speed_target < -MAX_MOTOR_SPEED) left_speed_target = -MAX_MOTOR_SPEED;
            if (right_speed_target > MAX_MOTOR_SPEED) right_speed_target = MAX_MOTOR_SPEED;
            if (right_speed_target < -MAX_MOTOR_SPEED) right_speed_target = -MAX_MOTOR_SPEED;
            break;

        case MOTOR_MODE_YAW_CORRECTION:
            // Yaw角闭环模式
            // 获取当前MPU6050的yaw值 (这里假设有一个全局变量或函数获取当前yaw)
            // 暂时使用一个临时值，实际应该从MPU6050获取
            float current_yaw = 0.0f; // TODO: 替换为实际的yaw获取函数
            yaw_correction = PID_Calculate(&g_motorControl.yaw_pid, current_yaw);

            left_speed_target = g_motorControl.base_speed - yaw_correction;
            right_speed_target = g_motorControl.base_speed + yaw_correction;
            
            // 限制速度目标值，避免过大
            if (left_speed_target > MAX_MOTOR_SPEED) left_speed_target = MAX_MOTOR_SPEED;
            if (left_speed_target < -MAX_MOTOR_SPEED) left_speed_target = -MAX_MOTOR_SPEED;
            if (right_speed_target > MAX_MOTOR_SPEED) right_speed_target = MAX_MOTOR_SPEED;
            if (right_speed_target < -MAX_MOTOR_SPEED) right_speed_target = -MAX_MOTOR_SPEED;
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

    // 限制PID输出值，防止电机跑满
    if (pwm_L > MAX_MOTOR_SPEED) pwm_L = MAX_MOTOR_SPEED;
    if (pwm_L < -MAX_MOTOR_SPEED) pwm_L = -MAX_MOTOR_SPEED;
    if (pwm_R > MAX_MOTOR_SPEED) pwm_R = MAX_MOTOR_SPEED;
    if (pwm_R < -MAX_MOTOR_SPEED) pwm_R = -MAX_MOTOR_SPEED;

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