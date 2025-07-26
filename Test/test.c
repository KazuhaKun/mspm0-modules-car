/*
 * test.c
 *
 *  模块化测试代码
 */
#include "test.h"
#include "oled_hardware_i2c.h"
#include "mpu6050.h"
#include "motor_control.h"
#include "Encoder.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// 用于在OLED上显示调试信息
char oled_buffer[40];

/**
 * @brief 在OLED上显示循迹传感器状态
 */
void Test_Display_LineTracker(void)
{
    LineTracker_ReadSensors();

    // 显示传感器原始值
    sprintf(oled_buffer, "L:%d%d%d%d%d%d%d",
            g_lineTracker.sensorValue[0], g_lineTracker.sensorValue[1],
            g_lineTracker.sensorValue[2], g_lineTracker.sensorValue[3],
            g_lineTracker.sensorValue[4], g_lineTracker.sensorValue[5],
            g_lineTracker.sensorValue[6]);
    OLED_ShowString(0, 0, (uint8_t *)oled_buffer, 16);

    // 显示线位置
    sprintf(oled_buffer, "Pos:%-4d", g_lineTracker.linePosition);
    OLED_ShowString(0, 2, (uint8_t *)oled_buffer, 16);

    // 显示状态
    const char* state_str = "Unk";
    switch(g_lineTracker.lineState) {
        case LINE_STATE_ON_LINE: state_str = "OnLine"; break;
        case LINE_STATE_LEFT_TURN: state_str = "Left"; break;
        case LINE_STATE_RIGHT_TURN: state_str = "Right"; break;
        case LINE_STATE_NO_LINE: state_str = "NoLine"; break;
        case LINE_STATE_ALL_LINE: state_str = "All"; break;
        default: break;
    }
    sprintf(oled_buffer, "ST: %s", state_str);
    OLED_ShowString(0, 4, (uint8_t *)oled_buffer, 16);
}

/**
 * @brief 在OLED上显示MPU6050 (IMU) 数据
 */
void Test_Display_IMU(void)
{
    // MPU6050的数据由中断服务程序在后台更新，这里直接使用全局变量
    sprintf(oled_buffer, "P:%.1f R:%.1f", pitch, roll);
    OLED_ShowString(0, 0, (uint8_t *)oled_buffer, 16);
    sprintf(oled_buffer, "Y:%.1f", yaw);
    OLED_ShowString(0, 2, (uint8_t *)oled_buffer, 16);
}

/**
 * @brief 在OLED上显示电机和编码器信息
 */
void Test_Display_Motor_Encoder(void)
{
    int16_t speed_L = Encoder_Get_Speed_L();
    int16_t speed_R = Encoder_Get_Speed_R();

    sprintf(oled_buffer, "L_S:%-4d R_S:%-4d", speed_L, speed_R);
    OLED_ShowString(0, 0, (uint8_t *)oled_buffer, 16);

    // 显示PID输出的PWM值 (需要从motor_control中获取，或在此处简化显示)
    // 假设我们显示目标速度
    sprintf(oled_buffer, "L_T:%-3.0f R_T:%-3.0f", g_motorControl.speed_pid_L.target, g_motorControl.speed_pid_R.target);
    OLED_ShowString(0, 2, (uint8_t *)oled_buffer, 16);
}

/**
 * @brief 测试手动模式下小车定速直线行驶
 */
void Test_Manual_Mode_Straight_Line(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Manual Mode", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Straight Line", 16);
    OLED_ShowString(0, 4, (uint8_t*)"Running...", 16);
    delay_ms(2000);
    
    // 设置为手动模式
    MotorControl_SetMode(MOTOR_MODE_MANUAL);
    
    // 设置基础速度为30
    float base_speed = 30.0f;
    MotorControl_SetBaseSpeed(base_speed);
    
    // 直接设置左右电机速度，使小车直线行驶
    // 在手动模式下，MotorControl_Update()函数会直接返回，不进行任何处理
    // 因此我们需要手动设置电机速度
    Motor_Set_Pwm(base_speed, base_speed);
    
    // 持续运行直到程序被中断
    while(1) {
        // 在OLED上显示当前状态
        OLED_Clear();
        OLED_ShowString(0, 0, (uint8_t*)"Manual Mode", 16);
        OLED_ShowString(0, 2, (uint8_t*)"Speed: 30", 16);
        OLED_ShowString(0, 4, (uint8_t*)"Running...", 16);
        delay_ms(100);
    }
}

/**
 * @brief 检查循迹传感器状态，用于调试传感器安装是否正确
 */
void Test_Check_Line_Sensors(void)
{
    char oled_buffer[40];
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Sensor Test", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Check if", 16);
    OLED_ShowString(0, 4, (uint8_t*)"inverted", 16);
    delay_ms(2000);
    
    while(1) {
        LineTracker_ReadSensors();
        
        // 显示传感器原始值
        sprintf(oled_buffer, "S:%d%d%d%d%d%d%d",
                g_lineTracker.sensorValue[0], g_lineTracker.sensorValue[1],
                g_lineTracker.sensorValue[2], g_lineTracker.sensorValue[3],
                g_lineTracker.sensorValue[4], g_lineTracker.sensorValue[5],
                g_lineTracker.sensorValue[6]);
        OLED_ShowString(0, 0, (uint8_t*)oled_buffer, 16);
        
        // 显示线位置
        sprintf(oled_buffer, "Pos:%-4d", g_lineTracker.linePosition);
        OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
        
        // 显示状态
        const char* state_str = "Unk";
        switch(g_lineTracker.lineState) {
            case LINE_STATE_ON_LINE: state_str = "OnLine"; break;
            case LINE_STATE_LEFT_TURN: state_str = "Left"; break;
            case LINE_STATE_RIGHT_TURN: state_str = "Right"; break;
            case LINE_STATE_NO_LINE: state_str = "NoLine"; break;
            case LINE_STATE_ALL_LINE: state_str = "All"; break;
            default: break;
        }
        sprintf(oled_buffer, "ST: %s", state_str);
        OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
        
        // 显示位图
        sprintf(oled_buffer, "Bits:0x%02X", g_lineTracker.sensorBits);
        OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
        
        delay_ms(100);
    }
}

/**
 * @brief 综合测试所有模块，并在OLED上分屏显示
 */
void Test_All_Modules(void)
{
    uint32_t tick = 0;
    MotorControl_SetBaseSpeed(5); // 进一步降低基础速度用于测试，原来是20
    MotorControl_SetMode(MOTOR_MODE_YAW_CORRECTION);

    while(1)
    {
        tick++;

        // !!注意!!
        // MotorControl_Update() 已由TIMA1定时器中断在后台自动调用，
        // 主循环中不再需要调用它。

        // OLED显示刷新，每100ms刷新一次，避免闪烁
        if (tick % 10 == 0) {
            OLED_Clear();

            // 分时显示不同模块信息
            uint8_t display_mode = (tick / 100) % 2; // 每1秒切换一次显示内容

            switch(display_mode) {
                case 0: // 显示IMU
                    OLED_ShowString(80, 6, (uint8_t*)"IMU", 16);
                    Test_Display_IMU();
                    break;
                case 1: // 显示电机和编码器
                    OLED_ShowString(70, 6, (uint8_t*)"Motor", 16);
                    Test_Display_Motor_Encoder();
                    break;
            }
        }


        delay_ms(10); // 主循环延时
    }
}

/**
 * @brief 基于Yaw角的直线行驶测试，当循迹传感器检测不到线时停车
 */
void Test_Yaw_Straight_Line_With_LineTracker(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Yaw Straight", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Line Stop", 16);
    delay_ms(2000);
    OLED_Clear();
    
    // 设置初始Yaw角为目标角度
    float initial_yaw = yaw;  // 获取当前Yaw角作为参考
    MotorControl_SetTargetYaw(initial_yaw);
    
    // 设置基础速度
    MotorControl_SetBaseSpeed(30.0f);
    
    // 设置为Yaw角闭环模式
    MotorControl_SetMode(MOTOR_MODE_YAW_CORRECTION);
    
    while(1) {
        // 读取循迹传感器数据
        LineTracker_ReadSensors();
        
        // 检查是否检测到线
        if (!LineTracker_IsLineDetected()) {
            // 没有检测到线，停车
            MotorControl_SetMode(MOTOR_MODE_STOP);
            
            // 在OLED上显示状态
            OLED_ShowString(0, 0, (uint8_t*)"No Line", 16);
            OLED_ShowString(0, 2, (uint8_t*)"Stopped", 16);
            OLED_ShowString(0, 4, (uint8_t*)"Yaw:", 16);
            char yaw_buffer[10];
            sprintf(yaw_buffer, "%.1f", yaw);
            OLED_ShowString(40, 4, (uint8_t*)yaw_buffer, 16);
        } else {
            // 检测到线，继续直行
            if (g_motorControl.mode != MOTOR_MODE_YAW_CORRECTION) {
                // 如果之前是停止状态，重新设置为Yaw角闭环模式
                MotorControl_SetTargetYaw(initial_yaw);
                MotorControl_SetBaseSpeed(30.0f);
                MotorControl_SetMode(MOTOR_MODE_YAW_CORRECTION);
            }
            
            // 在OLED上显示状态
            OLED_ShowString(0, 0, (uint8_t*)"Running", 16);
            OLED_ShowString(0, 2, (uint8_t*)"Line:Yes", 16);
            OLED_ShowString(0, 4, (uint8_t*)"Yaw:", 16);
            char yaw_buffer[10];
            sprintf(yaw_buffer, "%.1f", yaw);
            OLED_ShowString(40, 4, (uint8_t*)yaw_buffer, 16);
            OLED_ShowString(0, 6, (uint8_t*)"Target:", 16);
            char target_yaw_buffer[10];
            sprintf(target_yaw_buffer, "%.1f", initial_yaw);
            OLED_ShowString(60, 6, (uint8_t*)target_yaw_buffer, 16);
        }
        
        delay_ms(100);
    }
}
