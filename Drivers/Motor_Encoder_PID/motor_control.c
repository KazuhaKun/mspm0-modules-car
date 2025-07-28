/*
 * motor_control.c
 *
 *  电机控制实现
 */

#include "motor_control.h"
#include "Encoder.h"
#include "linetracker.h"

// 循迹PID控制器参数设置
// Kp: 比例增益，决定控制器对位置偏差的响应强度
// Ki: 积分增益，用于消除稳态误差，但在循迹应用中通常设为0
// Kd: 微分增益，预测偏差变化趋势，对循迹控制非常重要
#define LINE_PID_KP         0.8f   // 比例增益，影响循迹响应速度
#define LINE_PID_KI         0.0f   // 积分增益，循迹应用中通常为0
#define LINE_PID_KD         1.2f   // 微分增益，有助于预测偏差变化
#define LINE_PID_INT_LIMIT  100.0f // 积分限幅，防止积分饱和
#define LINE_PID_OUT_LIMIT  30.0f  // 输出限幅，限制最大修正值，防止急转

// Yaw角PID控制器参数设置
// Kp: 比例增益，决定控制器对当前误差的响应强度
// Ki: 积分增益，用于消除稳态误差
// Kd: 微分增益，预测系统未来趋势，减小超调
#define YAW_PID_KP          1.0f
#define YAW_PID_KI          0.0f
#define YAW_PID_KD          0.2f
#define YAW_PID_INT_LIMIT   200.0f  // 积分限幅，防止积分饱和
#define YAW_PID_OUT_LIMIT   100.0f  // 输出限幅，限制最大修正值

// 速度环PID控制器参数设置
// 增大Kp可提高响应速度，但可能导致系统不稳定
// 增大Ki可消除稳态误差，但可能导致系统震荡
// 增大Kd可改善动态响应，但对噪声敏感
#define SPEED_PID_KP        1.2f    // 增大比例增益以获得更快的响应速度
#define SPEED_PID_KI        0.2f    // 增大积分增益以消除稳态误差
#define SPEED_PID_KD        0.1f    // 增大微分增益以改善动态响应
#define SPEED_PID_INT_LIMIT 50.0f  // 速度环积分限幅
#define SPEED_PID_OUT_LIMIT 100.0f  // 速度环输出限幅

// 添加速度变化限制，避免速度突变导致电机震动或失步
#define MAX_SPEED_CHANGE 10.0f
// 添加最大速度限制，防止电机跑满导致失控
#define MAX_MOTOR_SPEED 30.0f

// 添加电机平衡因子，用于补偿左右电机速度差异
// 根据实际测试调整该值：>1.0表示右电机需要更多功率，<1.0表示左电机需要更多功率
#define MOTOR_BALANCE_FACTOR 1.0f

Motor_Control_t g_motorControl;

/**
 * @brief 初始化电机控制器
 * 
 * 该函数完成以下初始化工作：
 * 1. 初始化电机驱动模块
 * 2. 初始化编码器模块
 * 3. 初始化各个PID控制器（循迹、Yaw角控制、左右轮速度控制）
 * 4. 设置初始控制模式和参数
 */
void MotorControl_Init(void)
{
    // 初始化电机和编码器
    Motor_Init();
    Encoder_Init();
    LineTracker_Init();

    // 初始化PID控制器
    // 循迹PID控制器用于根据线位置偏差调整小车方向
    PID_Init(&g_motorControl.line_pid, LINE_PID_KP, LINE_PID_KI, LINE_PID_KD, LINE_PID_INT_LIMIT, LINE_PID_OUT_LIMIT);
    // Yaw角PID控制器用于保持或调整小车的朝向
    PID_Init(&g_motorControl.yaw_pid, YAW_PID_KP, YAW_PID_KI, YAW_PID_KD, YAW_PID_INT_LIMIT, YAW_PID_OUT_LIMIT);
    // 左轮速度PID控制器用于精确控制左轮转速
    PID_Init(&g_motorControl.speed_pid_L, SPEED_PID_KP, SPEED_PID_KI, SPEED_PID_KD, SPEED_PID_INT_LIMIT, SPEED_PID_OUT_LIMIT);
    // 右轮速度PID控制器用于精确控制右轮转速
    PID_Init(&g_motorControl.speed_pid_R, SPEED_PID_KP, SPEED_PID_KI, SPEED_PID_KD, SPEED_PID_INT_LIMIT, SPEED_PID_OUT_LIMIT);

    // 设置初始状态
    g_motorControl.mode = MOTOR_MODE_STOP;
    g_motorControl.base_speed = 20.0f; // 降低默认基础速度，避免过快
    g_motorControl.target_yaw = 0.0f;
    
    // 设置循迹PID的目标值为0（保持在线中央）
    PID_SetTarget(&g_motorControl.line_pid, 0.0f);
}

/**
 * @brief 设置电机控制模式
 * @param mode 新的控制模式
 * 
 * 控制模式决定了电机控制的策略：
 * - MOTOR_MODE_STOP: 停止所有电机运动
 * - MOTOR_MODE_LINE_FOLLOWING: 循迹模式
 * - MOTOR_MODE_YAW_CORRECTION: Yaw角闭环控制模式
 * - MOTOR_MODE_MANUAL: 手动控制模式
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
 * 
 * 基础速度是小车在直线行驶时的参考速度，
 * 在循迹或转向时会基于此值进行调整
 */
void MotorControl_SetBaseSpeed(float speed)
{
    // // 限制基础速度不能为负值
    // if (speed < 0.0f) {
    //     speed = 0.0f;
    // }
    g_motorControl.base_speed = speed;
}

/**
 * @brief 设置目标Yaw角
 * @param yaw 目标Yaw角
 * 
 * Yaw角是小车绕垂直轴旋转的角度，用于方向控制
 */
void MotorControl_SetTargetYaw(float yaw)
{
    g_motorControl.target_yaw = yaw;
    PID_SetTarget(&g_motorControl.yaw_pid, yaw);
}

/**
 * @brief 更新电机控制状态，应在主循环中定期调用
 * 
 * 这是电机控制的核心函数，根据当前控制模式计算并设置电机输出：
 * 1. 根据控制模式计算左右轮目标速度
 * 2. 使用编码器获取实际速度反馈
 * 3. 通过PID控制器计算速度控制输出
 * 4. 设置电机PWM驱动值
 */
void MotorControl_Update(void)
{
    float line_correction = 0;       // 循迹修正值
    float yaw_correction = 0;        // Yaw角修正值
    float left_speed_target = 0;    // 左轮目标速度
    float right_speed_target = 0;   // 右轮目标速度

    // 更新循迹传感器数据
    LineTracker_ReadSensors();
    
    // 根据控制模式计算目标速度
    switch (g_motorControl.mode) {
        case MOTOR_MODE_LINE_FOLLOWING:
            // 循迹模式 - 根据传感器检测到的线位置调整行驶方向
            // 获取当前线位置偏差值（-30到30，0表示在线中央）
            line_correction = PID_Calculate(&g_motorControl.line_pid, g_lineTracker.linePosition);
            
            // 根据线位置偏差计算左右轮速度差值
            // 使用比例控制方式，避免速度突变和反向
            // 将修正值转换为比例值(-1到1之间)
            float correction_ratio = line_correction / LINE_PID_OUT_LIMIT;
            
            // 按比例调整左右轮速度，避免出现负速度和速度突变
            left_speed_target = g_motorControl.base_speed * (1.0f - correction_ratio);
            right_speed_target = g_motorControl.base_speed * (1.0f + correction_ratio) * MOTOR_BALANCE_FACTOR;
            
            // 限制速度目标值，避免过大
            left_speed_target = left_speed_target > MAX_MOTOR_SPEED ? MAX_MOTOR_SPEED : (left_speed_target < 0) ? 0 : left_speed_target;
            right_speed_target = right_speed_target > MAX_MOTOR_SPEED ? MAX_MOTOR_SPEED : (right_speed_target < 0) ? 0 : right_speed_target;
            break;
            
        case MOTOR_MODE_YAW_CORRECTION:
            // Yaw角闭环模式 - 通过调整左右轮速度差实现转向控制
            // 获取当前MPU6050的yaw值 (这里假设有一个全局变量或函数获取当前yaw)
            // 暂时使用一个临时值，实际应该从MPU6050获取
            yaw_correction = PID_Calculate(&g_motorControl.yaw_pid, yaw);

            // 根据Yaw角误差计算左右轮速度差值
            // 左轮速度 = 基础速度 - Yaw修正值
            left_speed_target = g_motorControl.base_speed - yaw_correction;
            // 右轮速度 = (基础速度 + Yaw修正值) * 平衡因子
            // 平衡因子用于补偿左右电机性能差异
            right_speed_target = (g_motorControl.base_speed + yaw_correction) * MOTOR_BALANCE_FACTOR;
            
            // 限制速度目标值，避免过大
            left_speed_target = (left_speed_target > MAX_MOTOR_SPEED) ? MAX_MOTOR_SPEED : (left_speed_target < -MAX_MOTOR_SPEED) ? -MAX_MOTOR_SPEED : left_speed_target;
            right_speed_target = (right_speed_target > MAX_MOTOR_SPEED) ? MAX_MOTOR_SPEED : (right_speed_target < -MAX_MOTOR_SPEED) ? -MAX_MOTOR_SPEED : right_speed_target;


            break;

        case MOTOR_MODE_MANUAL:
            // 手动模式下，速度由外部直接设置，这里不处理
            left_speed_target = g_motorControl.base_speed;
            right_speed_target = g_motorControl.base_speed * MOTOR_BALANCE_FACTOR;
            break;

        case MOTOR_MODE_STOP:
        default:
            MotorControl_Stop();
            return;
    }

    // 速度闭环控制 - 使用编码器反馈实现精确速度控制
    int32_t current_speed_L = Encoder_GetSpeed_PPS(0);
    int32_t current_speed_R = Encoder_GetSpeed_PPS(1);

    // 设置左轮速度目标并计算PID输出
    PID_SetTarget(&g_motorControl.speed_pid_L, left_speed_target);
    float pwm_L = PID_Calculate(&g_motorControl.speed_pid_L, current_speed_L);

    // 设置右轮速度目标并计算PID输出
    PID_SetTarget(&g_motorControl.speed_pid_R, right_speed_target);
    float pwm_R = PID_Calculate(&g_motorControl.speed_pid_R, current_speed_R);

    // 双重保护：限制PID输出值，防止电机跑满
    // 使用三元运算符简化边界检查逻辑
    pwm_L = (pwm_L > MAX_MOTOR_SPEED) ? MAX_MOTOR_SPEED : ((pwm_L < -MAX_MOTOR_SPEED) ? -MAX_MOTOR_SPEED : pwm_L);
    pwm_R = (pwm_R > MAX_MOTOR_SPEED) ? MAX_MOTOR_SPEED : ((pwm_R < -MAX_MOTOR_SPEED) ? -MAX_MOTOR_SPEED : pwm_R);

    // 设置电机PWM驱动值
    Motor_Set_Pwm(pwm_L, pwm_R);
}

/**
 * @brief 停止电机
 * 
 * 紧急停止所有电机运动，并重置所有PID控制器状态
 */
void MotorControl_Stop(void)
{
    Motor_Stop(MOTOR_ALL);
    PID_Reset(&g_motorControl.line_pid);
    PID_Reset(&g_motorControl.yaw_pid);
    PID_Reset(&g_motorControl.speed_pid_L);
    PID_Reset(&g_motorControl.speed_pid_R);
}