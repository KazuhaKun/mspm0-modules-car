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
#include "clock.h"
#include "linetracker.h"
#include "turn_detection.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// 用于在OLED上显示调试信息
char oled_buffer[40];


#define SQUARE_LINE_SPEED 45.0f       // 正方形直线行驶速度
#define SQUARE_TURN_SPEED 4.0f       // 原地转弯速度
#define SQUARE_TURN_SETTLE_MS 50      // 转弯完成后稳定时间（从200ms减少到50ms）
#define SQUARE_TURN_PRECISION 3.0f    // 转弯精度（度）
#define SQUARE_TURN_TIMEOUT_MS 1000   // 转弯 超时时间（毫秒）

/**
 * @brief 执行基于循迹传感器反馈的转向
 * @param direction 转向方向（1=左转，-1=右转）
 * @return 转向结果（0=成功，-1=超时）
 */
static int PerformSensorBasedTurn(int direction) {
    uint32_t turn_start_time = tick_ms;
    
    // 设置转向模式：实现差速转向，一侧车轮正转，另一侧车轮反转
    MotorControl_SetMode(MOTOR_MODE_SPEED_CONTROL);
    
    if (direction > 0) {
        // 左转：左轮反向旋转，右轮正向旋转
        MotorControl_SetSpeedTarget(-SQUARE_TURN_SPEED * 0.5f, SQUARE_TURN_SPEED);
    } else {
        // 右转：左轮正向旋转，右轮反向旋转
        MotorControl_SetSpeedTarget(SQUARE_TURN_SPEED, -SQUARE_TURN_SPEED * 0.5f);
    }
    
    // 等待中间传感器检测到线或者超时
    while (1) {
        // 更新传感器数据
        LineTracker_ReadSensors();
        
        // 检查中间传感器是否检测到线（表示转向完成）
        if (g_lineTracker.sensorValue[2] && g_lineTracker.sensorValue[3]) {  // 传感器2和3同时检测到线才表示转向完成
            MotorControl_SetMode(MOTOR_MODE_STOP);
            return 0; // 成功
        }
        
        // 检查超时
        uint32_t turn_duration = tick_ms - turn_start_time;
        if (turn_duration > SQUARE_TURN_TIMEOUT_MS) {
            MotorControl_SetMode(MOTOR_MODE_STOP);
            return -1; // 超时
        }
        
        // 电机控制更新在中断中处理，这里只需要短暂延时以控制循环频率
        delay_ms(10);
    }
}

/**
 * @brief 正方形循迹 - 基于循迹传感器反馈的转向控制
 * 
 * 执行流程：
 * 1. 7路循迹直线行驶
 * 2. 检测到左侧转弯信号立即停车
 * 3. 使用循迹传感器反馈进行转向，直到中间传感器检测到线
 * 4. 等待稳定后继续直线行驶
 * 5. 重复4次后停止（一圈正方形）
 */
void Test_Square_Movement_Hybrid(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Running...", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Square", 16);
    
    // 状态变量
    typedef enum {
        SQUARE_STATE_LINE_FOLLOWING,    // 直线巡线状态
        SQUARE_STATE_SETTLING,          // 转弯后稳定状态
        SQUARE_STATE_COMPLETED          // 完成状态
    } Square_State_t;
    
    Square_State_t current_state = SQUARE_STATE_LINE_FOLLOWING;
    uint8_t completed_sides = 0;        // 已完成的边数
    uint32_t settle_start_time = 0;     // 稳定阶段开始时间
    
    // 设置初始循迹模式
    MotorControl_SetBaseSpeed(SQUARE_LINE_SPEED);
    MotorControl_SetMode(MOTOR_MODE_LINE_FOLLOWING);
    
    while(1) {
        switch(current_state) {
            case SQUARE_STATE_LINE_FOLLOWING:
            {
                // 直线巡线状态
                
                // 检查是否检测到转弯
                if (TurnDetection_IsTurnReady()) {
                    // 立即停车并执行转向
                    MotorControl_SetMode(MOTOR_MODE_STOP);
                    delay_ms(50); // 短暂暂停确保停车，从100ms减少到20ms以提高响应速度
                    
                    // 执行基于传感器反馈的左转
                    int turn_result = PerformSensorBasedTurn(1); // 1表示左转
                    
                    // 重置转弯检测状态
                    TurnDetection_Reset();
                    
                    settle_start_time = tick_ms;
                    current_state = SQUARE_STATE_SETTLING;
                } else {
                    // 正常直线巡线 - 简化显示
                    OLED_ShowString(0, 0, (uint8_t*)"Running...", 16);
                    sprintf(oled_buffer, "Side: %d/4", completed_sides + 1);
                    OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
                }
                break;
            }
            
            case SQUARE_STATE_SETTLING:
            {
                // 转弯后稳定状态
                
                if ((tick_ms - settle_start_time) >= SQUARE_TURN_SETTLE_MS) {
                    // 稳定时间到，继续下一边的直线行驶
                    completed_sides++;
                    
                    if (completed_sides >= 4) {  // 1圈 * 4边 = 4边
                        // 完成4边，进入完成状态
                        current_state = SQUARE_STATE_COMPLETED;
                    }
                    else {
                        // 继续下一边
                        MotorControl_SetBaseSpeed(SQUARE_LINE_SPEED);
                        MotorControl_SetMode(MOTOR_MODE_LINE_FOLLOWING);
                        current_state = SQUARE_STATE_LINE_FOLLOWING;
                    }
                }
                break;
            }
            
            case SQUARE_STATE_COMPLETED:
            {
                // 完成状态
                MotorControl_SetMode(MOTOR_MODE_STOP);
                return; // 函数结束
            }
        }
        
        // 显示传感器状态（底部，仅在非完成状态）
        if (current_state != SQUARE_STATE_COMPLETED) {
            sprintf(oled_buffer, "S:%d%d%d%d%d%d%d",
                    g_lineTracker.sensorValue[0], g_lineTracker.sensorValue[1],
                    g_lineTracker.sensorValue[2], g_lineTracker.sensorValue[3],
                    g_lineTracker.sensorValue[4], g_lineTracker.sensorValue[5],
                    g_lineTracker.sensorValue[6]);
            OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
        }
        
        delay_ms(20); // 控制循环频率，提高响应速度
    }
}

/**
 * @brief 指定圈数的正方形循迹 - 基于循迹传感器反馈的转向控制
 * @param laps 正方形圈数 (1-5)
 */
void Test_Square_Movement_Hybrid_With_Laps(int laps) {
    // 确保laps在有效范围内
    if (laps < 1) laps = 1;
    if (laps > 5) laps = 5;
    
    // Test_Square_Movement_Hybrid执行一次就是一圈(4条边)
    for (int i = 0; i < laps; i++) {
        Test_Square_Movement_Hybrid();
    }
    
    // 完成所有圈数后显示完成信息
    OLED_Clear();
    sprintf(oled_buffer, "%d laps done!", laps);
    OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
    OLED_ShowString(0, 4, (uint8_t*)"Press key exit", 16);
    
    // 等待按键退出
    while(1) {
        // 检查按键状态
        if (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_23) ||  // Key_1
            !DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_21) ||  // Key_2
            !DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_18) ||  // Key_3
            !DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_17)) {  // Key_4
            break;
        }
        delay_ms(10);
    }
}

/**
 * @brief 通过按键设置并执行指定圈数的正方形循迹
 * Key1: 增加圈数
 * Key2: 减少圈数
 * Key3: 确认并开始执行
 */
void Test_Square_Movement_Hybrid_Key_Control(void) {
    int laps = 1; // 默认圈数
    
    // 显示初始界面
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Set laps:", 16);
    sprintf(oled_buffer, "Laps: %d (1-5)", laps);
    OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
    OLED_ShowString(0, 4, (uint8_t*)"Key1:+ Key2:-", 16);
    OLED_ShowString(0, 6, (uint8_t*)"Key3:Start", 16);
    
    // 按键控制圈数设置
    while (1) {
        // 检查Key1: 增加圈数
        if (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_23)) {
            delay_ms(20); // 消除抖动
            if (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_23)) { // 再次确认按键状态
                while (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_23)); // 等待按键释放
                laps++;
                if (laps > 5) laps = 5;
                sprintf(oled_buffer, "Laps: %d (1-5)", laps);
                OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
            }
        }
        
        // 检查Key2: 减少圈数
        else if (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_21)) {
            delay_ms(20); // 消除抖动
            if (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_21)) { // 再次确认按键状态
                while (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_21)); // 等待按键释放
                laps--;
                if (laps < 1) laps = 1;
                sprintf(oled_buffer, "Laps: %d (1-5)", laps);
                OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
            }
        }
        
        // 检查Key3: 确认并开始执行
        else if (!DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_18)) {
            delay_ms(20); // 消除抖动
            if (!DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_18)) { // 再次确认按键状态
                while (!DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_18)); // 等待按键释放
                break; // 退出设置循环，开始执行
            }
        }
        
        delay_ms(50);
    }
    
    // 开始执行指定圈数的正方形运动
    Test_Square_Movement_Hybrid_With_Laps(laps);
}

/**
 * @brief 循迹传感器调试显示函数
 * 实时显示7路循迹传感器的状态
 */
void Test_Line_Sensors_Debug(void) {
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Line Sensor", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Debug Test", 16);
    delay_ms(1000);
    OLED_Clear();
    
    while (1) {
        // 读取传感器数据
        LineTracker_ReadSensors();
        
        // 显示传感器状态
        OLED_ShowString(0, 0, (uint8_t*)"Sensors:", 16);
        sprintf(oled_buffer, "S1:%d S2:%d S3:%d", 
                g_lineTracker.sensorValue[0],
                g_lineTracker.sensorValue[1], 
                g_lineTracker.sensorValue[2]);
        OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
        
        sprintf(oled_buffer, "S4:%d S5:%d S6:%d", 
                g_lineTracker.sensorValue[3],
                g_lineTracker.sensorValue[4], 
                g_lineTracker.sensorValue[5]);
        OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
        
        sprintf(oled_buffer, "S7:%d", g_lineTracker.sensorValue[6]);
        OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
        
        // 检查是否有按键按下退出
        if (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_23) ||  // Key_1
            !DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_21) ||  // Key_2
            !DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_18) ||  // Key_3
            !DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_17)) {  // Key_4
            
            // 等待按键释放
            while (!DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_23) ||  // Key_1
                   !DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_21) ||  // Key_2
                   !DL_GPIO_readPins(GPIOB, DL_GPIO_PIN_18) ||  // Key_3
                   !DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_17));   // Key_4
            
            break;
        }
        
        delay_ms(50); // 控制刷新频率
    }
    
    // 退出提示
    OLED_Clear();
    OLED_ShowString(0, 2, (uint8_t*)"Exit Sensor", 16);
    OLED_ShowString(0, 4, (uint8_t*)"Debug Test", 16);
    delay_ms(1000);
}
