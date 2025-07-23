/*
 * test.c - 电机测试模块实现
 */

#include "test.h"
#include "main.h"
#include "ti_msp_dl_config.h"

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
    static uint32_t safety_check_time = 0;
    uint32_t current_time;
    mspm0_get_clock_ms(&current_time);
    
    if (current_time - safety_check_time > 1000) {  // 每1秒检查一次
        MotorControl_Manual(MOTOR_CONTROL_LEFT, 1.0f);   // 重新确保左电机满速
        MotorControl_Manual(MOTOR_CONTROL_RIGHT, 1.0f);  // 重新确保右电机满速
        safety_check_time = current_time;
    }
    
    // 显示编码器数据
    Test_DisplayEncoderData();
    mspm0_delay_ms(200);  // 每200ms更新一次显示
}

// 显示编码器数据函数
void Test_DisplayEncoderData(void)
{
    OLED_Clear();
    
    // 获取两个电机的编码器原始数据
    motor_test.left_encoder_count = MotorControl_GetEncoderCount(MOTOR_CONTROL_LEFT);
    motor_test.left_current_speed = MotorControl_GetCurrentSpeed(MOTOR_CONTROL_LEFT);
    motor_test.right_encoder_count = MotorControl_GetEncoderCount(MOTOR_CONTROL_RIGHT);
    motor_test.right_current_speed = MotorControl_GetCurrentSpeed(MOTOR_CONTROL_RIGHT);
    
    // 计算转速 (基于390脉冲/转，4倍频 = 1560脉冲/转)
    float left_rpm = (motor_test.left_current_speed / 1560.0f) * 60.0f;
    float right_rpm = (motor_test.right_current_speed / 1560.0f) * 60.0f;
    
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
    
    // 第二行：PPS (脉冲每秒)
    OLED_ShowNum(0, 2, (uint32_t)motor_test.left_current_speed, 4, 16);
    OLED_ShowNum(64, 2, (uint32_t)motor_test.right_current_speed, 4, 16);
    
    // 第三行：RPM (转每分钟)
    OLED_ShowNum(0, 4, (uint32_t)left_rpm, 4, 16);
    OLED_ShowNum(64, 4, (uint32_t)right_rpm, 4, 16);
    
    // 第四行：速度差异
    int32_t speed_diff = motor_test.left_current_speed - motor_test.right_current_speed;
    if (speed_diff >= 0) {
        OLED_ShowNum(0, 6, (uint32_t)speed_diff, 5, 16);
    } else {
        OLED_ShowString(0, 6, (uint8_t*)"-", 16);
        OLED_ShowNum(8, 6, (uint32_t)(-speed_diff), 4, 16);
    }
}
