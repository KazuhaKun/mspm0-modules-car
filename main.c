/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
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

    // 3. 在OLED上显示启动信息
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"All modules", 16);
    OLED_ShowString(0, 2, (uint8_t*)"initialized.", 16);
    OLED_ShowString(0, 4, (uint8_t*)"Starting test...", 16);
    delay_ms(2000); // 等待2秒

    // 4. 调用传感器检测测试函数
    // 这个函数包含一个无限循环，会接管程序的所有测试和显示任务
    // Test_Check_Line_Sensors();


    // 主循环
    while (1) 
    {
        // Test_DualMotorLoop();    //双电机测试
        // Test_All_Modules();      //综合测试
        // Test_Manual_Mode_Straight_Line(); //手动模式直线行驶测试
        Test_LineTracker(); //基于7路循迹传感器的行驶测试，当循迹传感器检测不到线时停车
        // Test_Yaw_Straight_Line_With_LineTracker(); //基于Yaw角直线行驶，循迹传感器检测不到线时停车
    }
}