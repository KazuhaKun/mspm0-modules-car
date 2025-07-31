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


#define SQUARE_LINE_SPEED 30.0f       // 正方形直线行驶速度
#define SQUARE_TURN_SPEED 5.0f       // 原地转弯速度
#define SQUARE_TURN_SETTLE_MS 200     // 转弯完成后稳定时间
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
        if (g_lineTracker.sensorValue[2]) {  // 传感器2是中间偏左传感器
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
        delay_ms(5); // 从10ms减少到5ms以提高响应速度
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
 * 5. 重复4次后停止
 */
void Test_Square_Movement_Hybrid(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Square Hybrid", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Line+Turn", 16);
    delay_ms(1000);
    OLED_Clear();
    
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
                    delay_ms(100); // 短暂暂停确保停车
                    
                    // 执行基于传感器反馈的左转
                    int turn_result = PerformSensorBasedTurn(1); // 1表示左转
                    
                    // 重置转弯检测状态
                    TurnDetection_Reset();
                    
                    // 显示转弯结果
                    OLED_ShowString(0, 0, (uint8_t*)"Turn Result", 16);
                    switch(turn_result) {
                        case 0: OLED_ShowString(0, 2, (uint8_t*)"SUCCESS", 16); break;
                        case -1: OLED_ShowString(0, 2, (uint8_t*)"TIMEOUT", 16); break;
                        default: OLED_ShowString(0, 2, (uint8_t*)"UNKNOWN", 16); break;
                    }
                    delay_ms(500);
                    
                    settle_start_time = tick_ms;
                    current_state = SQUARE_STATE_SETTLING;
                } else {
                    // 正常直线巡线 - 显示详细传感器状态
                    OLED_ShowString(0, 0, (uint8_t*)"Line Follow", 16);
                    sprintf(oled_buffer, "Side: %d/16", completed_sides + 1);
                    OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
                    sprintf(oled_buffer, "L_cnt:%d", g_turnDetection.left_sensor_count);
                    OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
                    
                    // 显示左侧传感器具体状态 (0,1,2对应左侧3个传感器)
                    sprintf(oled_buffer, "L0:%d L1:%d L2:%d", 
                        g_lineTracker.sensorValue[0] ? 1 : 0,
                        g_lineTracker.sensorValue[1] ? 1 : 0,
                        g_lineTracker.sensorValue[2] ? 1 : 0);
                    OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
                    
                    // 显示抑制状态
                    if (g_turnDetection.state == TURN_STATE_INHIBITED) {
                        uint32_t inhibit_remaining = TURN_INHIBIT_TIME_MS - (tick_ms - g_turnDetection.inhibit_start_time);
                        sprintf(oled_buffer, "Inhib:%dms", (int)inhibit_remaining);
                        OLED_ShowString(64, 0, (uint8_t*)oled_buffer, 16);
                    } else {
                        OLED_ShowString(64, 0, (uint8_t*)"Ready", 16);
                    }
                }
                break;
            }
            
            case SQUARE_STATE_SETTLING:
            {
                // 转弯后稳定状态
                
                if ((tick_ms - settle_start_time) >= SQUARE_TURN_SETTLE_MS) {
                    // 稳定时间到，继续下一边的直线行驶
                    completed_sides++;
                    
                    if (completed_sides >= 16) {  // 4圈 * 4边 = 16边
                        // 完成16边，进入完成状态
                        current_state = SQUARE_STATE_COMPLETED;
                    }
                    else {
                        // 继续下一边
                        MotorControl_SetBaseSpeed(SQUARE_LINE_SPEED);
                        MotorControl_SetMode(MOTOR_MODE_LINE_FOLLOWING);
                        current_state = SQUARE_STATE_LINE_FOLLOWING;
                    }
                    
                    OLED_ShowString(0, 0, (uint8_t*)"Resume", 16);
                    OLED_ShowString(0, 2, (uint8_t*)"Line Follow", 16);
                    sprintf(oled_buffer, "Side: %d/16", completed_sides + 1);
                    OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
                }
                else {
                    // 继续稳定等待
                    sprintf(oled_buffer, "Wait:%dms", (int)(SQUARE_TURN_SETTLE_MS - (tick_ms - settle_start_time)));
                    OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
                }
                break;
            }
            
            case SQUARE_STATE_COMPLETED:
            {
                // 完成状态
                MotorControl_SetMode(MOTOR_MODE_STOP);
                
                OLED_ShowString(0, 0, (uint8_t*)"Square", 16);
                OLED_ShowString(0, 2, (uint8_t*)"Completed!", 16);
                OLED_ShowString(0, 4, (uint8_t*)"16/16 sides", 16);
                OLED_ShowString(0, 6, (uint8_t*)"FINISHED", 16);
                
                // 保持完成状态，不再执行其他操作
                break;
            }
        }
        
        // 显示传感器状态（底部，仅在非完成状态）
        if (current_state != SQUARE_STATE_COMPLETED) {
            sprintf(oled_buffer, "S:%d%d%d%d%d%d%d",
                    g_lineTracker.sensorValue[0], g_lineTracker.sensorValue[1],
                    g_lineTracker.sensorValue[2], g_lineTracker.sensorValue[3],
                    g_lineTracker.sensorValue[4], g_lineTracker.sensorValue[5],
                    g_lineTracker.sensorValue[6]);
            // 根据当前状态选择显示位置，避免覆盖重要信息
            if (current_state == SQUARE_STATE_LINE_FOLLOWING) {
                OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
            }
        }
        
        delay_ms(5); // 控制循环频率，从20ms减少到5ms以提高响应速度
    }
}
