/*
 * test.c - 电机测试模块实现
 */

#include "test.h"
#include "main.h"
#include "ti_msp_dl_config.h"
#include "linetracker.h"
#include "Encoder.h"

// 全局测试数据实例
MotorTest_t motor_test = {0};

// 测试模块初始化
void Test_Init(void)
{
    // 初始化电机控制系统
    MotorControl_Init();
    
    // 完全禁用PID控制，使用手动模式
    MotorControl_DisablePID_All();
    
    // 确保所有电机完全停止
    MotorControl_Stop_All();
    mspm0_delay_ms(500);  // 等待500ms确保电机停止
    
    // 初始化测试数据
    motor_test.last_update_time = 0;
    mspm0_get_clock_ms(&motor_test.last_update_time);
}

// 开始双电机测试
void Test_DualMotorStart(void)
{
    // 清屏并显示测试开始信息
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Dual Motor Test", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Both Motors", 16);
    mspm0_delay_ms(2000);
    
    // 使用手动控制模式，同时设置两个电机满速（不使用PID）
    MotorControl_Manual(MOTOR_CONTROL_LEFT, 1.0f);   // 左电机满速正转 (1.0 = 100%功率)
    MotorControl_Manual(MOTOR_CONTROL_RIGHT, 1.0f);  // 右电机满速正转 (1.0 = 100%功率)
}

// 双电机测试主循环
void Test_DualMotorLoop(void)
{
    // 定期重新确保两个电机保持满速（防止意外变化）
    static unsigned long safety_check_time = 0;
    unsigned long current_time;
    mspm0_get_clock_ms(&current_time);
    
    if (current_time - safety_check_time > 1000) {  // 每1秒检查一次
        MotorControl_Manual(MOTOR_CONTROL_LEFT, 1.0f);   // 重新确保左电机满速
        MotorControl_Manual(MOTOR_CONTROL_RIGHT, 1.0f);  // 重新确保右电机满速
        safety_check_time = current_time;
    }
    
    // 显示编码器数据
    Test_DisplayEncoderData();
    mspm0_delay_ms(100);  // 每100ms更新一次显示，与速度计算频率匹配
}

// 显示编码器数据函数
void Test_DisplayEncoderData(void)
{
    OLED_Clear();
    
    // 获取两个电机的编码器原始数据
    motor_test.left_encoder_count = MotorControl_GetEncoderCount(MOTOR_CONTROL_LEFT);
    motor_test.right_encoder_count = MotorControl_GetEncoderCount(MOTOR_CONTROL_RIGHT);
    
    // 直接从编码器获取速度（更可靠）
    motor_test.left_current_speed = Encoder_GetSpeed_PPS(MOTOR_CONTROL_LEFT);
    motor_test.right_current_speed = Encoder_GetSpeed_PPS(MOTOR_CONTROL_RIGHT);
    
    // 计算转速 (基于13ppr编码器，当前配置为2倍频 = 26脉冲/转)
    // 13ppr × 2倍频(A1+B1上升沿) = 26脉冲/转
    // RPS = PPS / 26脉冲每转
    float left_rps = motor_test.left_current_speed / 26.0f;
    float right_rps = motor_test.right_current_speed / 26.0f;
    
    // OLED显示格式：
    // 第一行：编码器累计计数 (左电机 | 右电机)
    // 第二行：PPS - 脉冲每秒 (左电机 | 右电机)  
    // 第三行：RPS - 转每秒 (左电机 | 右电机)
    // 第四行：速度差异
    
    // 第一行：编码器累计计数 (左电机 空格 右电机)
    if (motor_test.left_encoder_count >= 0) {
        OLED_ShowNum(0, 0, (uint32_t)motor_test.left_encoder_count, 5, 16);
    } else {
        OLED_ShowString(0, 0, (uint8_t*)"-", 16);
        OLED_ShowNum(8, 0, (uint32_t)(-motor_test.left_encoder_count), 4, 16);
    }
    
    if (motor_test.right_encoder_count >= 0) {
        OLED_ShowNum(64, 0, (uint32_t)motor_test.right_encoder_count, 5, 16);
    } else {
        OLED_ShowString(64, 0, (uint8_t*)"-", 16);
        OLED_ShowNum(72, 0, (uint32_t)(-motor_test.right_encoder_count), 4, 16);
    }
    
    // 第二行：PPS (脉冲每秒) - 处理正负值
    if (motor_test.left_current_speed >= 0) {
        OLED_ShowNum(0, 2, (uint32_t)motor_test.left_current_speed, 4, 16);
    } else {
        OLED_ShowString(0, 2, (uint8_t*)"-", 16);
        OLED_ShowNum(8, 2, (uint32_t)(-motor_test.left_current_speed), 3, 16);
    }
    
    if (motor_test.right_current_speed >= 0) {
        OLED_ShowNum(64, 2, (uint32_t)motor_test.right_current_speed, 4, 16);
    } else {
        OLED_ShowString(64, 2, (uint8_t*)"-", 16);
        OLED_ShowNum(72, 2, (uint32_t)(-motor_test.right_current_speed), 3, 16);
    }
    
    // 第三行：RPS (转每秒) - 处理正负值
    if (left_rps >= 0) {
        OLED_ShowNum(0, 4, (uint32_t)left_rps, 3, 16);
    } else {
        OLED_ShowString(0, 4, (uint8_t*)"-", 16);
        OLED_ShowNum(8, 4, (uint32_t)(-left_rps), 2, 16);
    }
    
    if (right_rps >= 0) {
        OLED_ShowNum(64, 4, (uint32_t)right_rps, 3, 16);
    } else {
        OLED_ShowString(64, 4, (uint8_t*)"-", 16);
        OLED_ShowNum(72, 4, (uint32_t)(-right_rps), 2, 16);
    }
    
    // 第四行：速度差异
    int32_t speed_diff = motor_test.left_current_speed - motor_test.right_current_speed;
    if (speed_diff >= 0) {
        OLED_ShowNum(0, 6, (uint32_t)speed_diff, 5, 16);
    } else {
        OLED_ShowString(0, 6, (uint8_t*)"-", 16);
        OLED_ShowNum(8, 6, (uint32_t)(-speed_diff), 4, 16);
    }
}

// ======================== 循迹测试功能 ========================

// 循迹测试初始化
void Test_LineTrackerInit(void)
{
    // 初始化循迹传感器
    LineTracker_Init();
    
    // 显示初始化信息
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"LineTracker", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Test Init", 16);
    mspm0_delay_ms(2000);
    
    printf("循迹测试初始化完成\n");
}

// 循迹测试主循环
void Test_LineTrackerLoop(void)
{
    // 读取传感器数据
    LineTracker_ReadSensors();
    
    // 显示循迹状态
    Test_LineTrackerDisplay();
    
    // 打印详细状态到串口（每500ms一次）
    static unsigned long last_print_time = 0;
    unsigned long current_time;
    mspm0_get_clock_ms(&current_time);
    
    if (current_time - last_print_time > 500) {
        // LineTracker_PrintStatus();    // 调试功能已注释
        last_print_time = current_time;
    }
    
    mspm0_delay_ms(100);  // 100ms更新一次
}

// 循迹状态显示
void Test_LineTrackerDisplay(void)
{
    char buffer[16];
    uint8_t i;
    
    OLED_Clear();
    
    // 第一行：标题
    OLED_ShowString(0, 0, (uint8_t*)"LineTracker", 16);
    
    // 第二行：传感器状态 (7个数字)
    for (i = 0; i < LINE_SENSOR_COUNT; i++) {
        OLED_ShowNum(i * 16, 2, g_lineTracker.sensorValue[i], 1, 16);
    }
    
    // 第三行：位置和激活数量
    if (g_lineTracker.linePosition >= 0) {
        OLED_ShowNum(0, 4, (uint32_t)g_lineTracker.linePosition, 3, 16);
    } else {
        OLED_ShowString(0, 4, (uint8_t*)"-", 16);
        OLED_ShowNum(8, 4, (uint32_t)(-g_lineTracker.linePosition), 2, 16);
    }
    
    OLED_ShowString(48, 4, (uint8_t*)"A:", 16);
    OLED_ShowNum(64, 4, g_lineTracker.activeSensorCount, 1, 16);
    
    // 第四行：状态文字
    const char* stateStr;
    switch (g_lineTracker.lineState) {
        case LINE_STATE_ON_LINE:    stateStr = "ON_LINE"; break;
        case LINE_STATE_LEFT_TURN:  stateStr = "LEFT"; break;
        case LINE_STATE_RIGHT_TURN: stateStr = "RIGHT"; break;
        case LINE_STATE_SHARP_LEFT: stateStr = "S_LEFT"; break;
        case LINE_STATE_SHARP_RIGHT:stateStr = "S_RIGHT"; break;
        case LINE_STATE_NO_LINE:    stateStr = "NO_LINE"; break;
        case LINE_STATE_ALL_LINE:   stateStr = "ALL_LINE"; break;
        default:                    stateStr = "UNKNOWN"; break;
    }
    OLED_ShowString(0, 6, (uint8_t*)stateStr, 16);
}
