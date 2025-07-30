/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 *
 * Redistribution a        // 测试转弯角度计算逻辑（调试转弯180度问题）
        // Test_Turn_Angle_Calculation();
        
        // 正方形循迹主功能（优化版 - 改进转弯检测）
        Test_Square_Movement_Hybrid();  // 混合模式：7路循迹直线 + MPU6050精确转弯e in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "main.h"
#include "test.h"
// #include "Test1.h"  // 已归档为Test1.h.bak

int main(void)
{
    // 系统初始化
    SYSCFG_DL_init();
    SysTick_Init();

    // 外设初始化
    MPU6050_Init();
    // delay_ms(8000);
    OLED_Init();
    // Ultrasonic_Init();
    // BNO08X_Init();
    // WIT_Init();
    // VL53L0X_Init();
    // LSM6DSV16X_Init();
    LineTracker_Init();
    MotorControl_Init(); // 此函数会初始化电机、编码器和所有PID控制器
    TurnDetection_Init(); // 初始化转弯检测模块

    // 3. 在OLED上显示启动信息
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"All modules", 16);
    OLED_ShowString(0, 2, (uint8_t*)"initialized.", 16);
    OLED_ShowString(0, 4, (uint8_t*)"Starting test...", 16);
    delay_ms(2000); // 等待2秒

    // 4. 调用传感器检测测试函数
    // 此函数用于调试目的，显示传感器的原始值
    // Test_Check_Line_Sensors();


    // 主循环
    while (1) 
    {
        // 选择要运行的测试函数：
        // 1. 取消注释对应的函数来运行特定测试
        // 2. 同时注释掉其他测试函数
        
        // 传感器状态检查（调试用）
        // Test_Check_Line_Sensors();  
        
        // 显示MPU6050 yaw角原始信息（调试陀螺仪）
        // Test_Display_Yaw_Info();
        
        // 测试MPU6050角度跨越特性（专门调试±180度边界）
        // Test_Angle_Crossing_Debug();
        
        // 测试转弯角度计算逻辑（调试转弯180度问题）
        // Test_Turn_Angle_Calculation();
        
        // 正方形循迹主功能（默认运行）
        Test_Square_Movement_Hybrid();  // 混合模式：7路循迹直线 + MPU6050精确转弯
    }
}