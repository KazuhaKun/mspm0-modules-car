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
#include <stdio.h>
#include <string.h>
#include <math.h>

// 用于在OLED上显示调试信息
char oled_buffer[40];


#define SQUARE_LINE_SPEED 15.0f       // 正方形直线行驶速度
#define SQUARE_TURN_SPEED 12.0f       // 原地转弯速度
#define SQUARE_TURN_ANGLE 90.0f       // 转弯角度（度）
#define SQUARE_TURN_SETTLE_MS 500     // 转弯完成后稳定时间
#define SQUARE_TURN_PRECISION 3.0f    // 转弯精度（度）
#define SQUARE_TURN_TIMEOUT_MS 3000   // 转弯超时时间（毫秒）
#define SQUARE_MIN_TURN_ANGLE 85.0f   // 最小转弯角度（防止转弯不足）
#define SQUARE_MAX_TURN_ANGLE 95.0f   // 最大转弯角度（防止转弯过度）

/**
 * @brief 正方形循迹 - 分解为直线巡线和定点转弯
 * 
 * 执行流程：
 * 1. 7路循迹直线行驶
 * 2. 检测到左侧转弯信号立即停车
 * 3. 使用MPU6050基准角度，原地逆时针转弯90度
 * 4. 等待500ms稳定后继续直线行驶
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
        SQUARE_STATE_TURNING,           // 转弯状态
        SQUARE_STATE_SETTLING,          // 转弯后稳定状态
        SQUARE_STATE_COMPLETED          // 完成状态
    } Square_State_t;
    
    Square_State_t current_state = SQUARE_STATE_LINE_FOLLOWING;
    uint8_t completed_sides = 0;        // 已完成的边数
    float turn_start_yaw = 0.0f;        // 转弯开始时的yaw角
    float target_yaw = 0.0f;            // 目标yaw角
    uint32_t turn_start_time = 0;       // 转弯开始时间
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
                    // 立即停车并进入转弯状态
                    MotorControl_SetMode(MOTOR_MODE_STOP);
                    
                    // 记录当前yaw角作为转弯基准
                    turn_start_yaw = yaw;
                    
                    // 计算目标角度
                    target_yaw = turn_start_yaw + SQUARE_TURN_ANGLE;
                    
                    // 将目标角度归一化到[-180, 180]范围
                    while (target_yaw > 180.0f) target_yaw -= 360.0f;
                    while (target_yaw < -180.0f) target_yaw += 360.0f;
                    
                    turn_start_time = tick_ms;
                    current_state = SQUARE_STATE_TURNING;
                    
                    // 重置转弯检测状态
                    TurnDetection_Reset();
                    
                    // 显示转弯信息
                    OLED_ShowString(0, 0, (uint8_t*)"Turning", 16);
                    sprintf(oled_buffer, "Side: %d/4", completed_sides + 1);
                    OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
                    sprintf(oled_buffer, "From:%.1f", turn_start_yaw);
                    OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
                    sprintf(oled_buffer, "To:%.1f", target_yaw);
                    OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
                } else {
                    // 正常直线巡线 - 显示详细传感器状态
                    OLED_ShowString(0, 0, (uint8_t*)"Line Follow", 16);
                    sprintf(oled_buffer, "Side: %d/4", completed_sides + 1);
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
            
            case SQUARE_STATE_TURNING:
            {
                // 转弯状态 - 使用MPU6050进行精确角度控制
                
                // 计算当前角度差 (target - current)
                float angle_diff = target_yaw - yaw;
                
                // 处理角度跨越±180度的情况 - 选择最短路径
                if (angle_diff > 180.0f) angle_diff -= 360.0f;
                if (angle_diff < -180.0f) angle_diff += 360.0f;
                
                // 计算实际已转弯的角度（相对于转弯开始时）
                float actual_turned = yaw - turn_start_yaw;
                // 同样处理角度跨越
                if (actual_turned > 180.0f) actual_turned -= 360.0f;
                if (actual_turned < -180.0f) actual_turned += 360.0f;
                
                // 检查转弯超时
                uint32_t turn_duration = tick_ms - turn_start_time;
                bool turn_timeout = (turn_duration > SQUARE_TURN_TIMEOUT_MS);
                
                // 检查转弯是否过度 - 基于实际转弯角度
                bool turn_excessive = (fabs(actual_turned) > SQUARE_MAX_TURN_ANGLE);
                
                // 显示转弯状态（包含实际转弯角度）
                OLED_ShowString(0, 0, (uint8_t*)"Turning", 16);
                sprintf(oled_buffer, "Cur:%.1f", yaw);
                OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
                sprintf(oled_buffer, "Tgt:%.1f", target_yaw);
                OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
                sprintf(oled_buffer, "Act:%.1f", actual_turned);
                OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
                
                // 显示剩余角度差在右侧
                sprintf(oled_buffer, "Diff:%.1f", angle_diff);
                OLED_ShowString(64, 0, (uint8_t*)oled_buffer, 16);
                
                // 显示转弯进度
                float turn_progress = (fabs(actual_turned) / SQUARE_TURN_ANGLE) * 100.0f;
                if (turn_progress > 100.0f) turn_progress = 100.0f;
                sprintf(oled_buffer, "Prog:%.0f%%", turn_progress);
                OLED_ShowString(64, 2, (uint8_t*)oled_buffer, 16);
                
                // 检查转弯完成条件 - 改进的判断逻辑
                bool angle_reached = (fabs(angle_diff) < SQUARE_TURN_PRECISION);
                bool min_angle_met = (fabs(actual_turned) >= SQUARE_MIN_TURN_ANGLE);
                bool turn_complete = angle_reached && min_angle_met;
                
                if (turn_complete || turn_timeout || turn_excessive) {
                    // 转弯完成、超时或过度，停止电机并进入稳定状态
                    MotorControl_SetMode(MOTOR_MODE_STOP);
                    settle_start_time = tick_ms;
                    current_state = SQUARE_STATE_SETTLING;
                    
                    if (turn_timeout) {
                        OLED_ShowString(64, 4, (uint8_t*)"TIMEOUT", 16);
                    } else if (turn_excessive) {
                        OLED_ShowString(64, 4, (uint8_t*)"EXCESS", 16);
                    } else {
                        OLED_ShowString(64, 4, (uint8_t*)"DONE", 16);
                    }
                }
                else {
                    // 继续转弯 - 原地逆时针转弯
                    MotorControl_SetMode(MOTOR_MODE_SPEED_CONTROL);
                    // 逆时针转弯：左轮反转，右轮正转
                    MotorControl_SetSpeedTarget(-SQUARE_TURN_SPEED, SQUARE_TURN_SPEED);
                }
                break;
            }
            
            case SQUARE_STATE_SETTLING:
            {
                // 转弯后稳定状态
                
                if ((tick_ms - settle_start_time) >= SQUARE_TURN_SETTLE_MS) {
                    // 稳定时间到，继续下一边的直线行驶
                    completed_sides++;
                    
                    if (completed_sides >= 4) {
                        // 完成4边，进入完成状态
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
                    sprintf(oled_buffer, "Side: %d/4", completed_sides + 1);
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
                OLED_ShowString(0, 4, (uint8_t*)"4/4 sides", 16);
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
        
        delay_ms(20); // 控制循环频率，提高响应速度
    }
}

/**
 * @brief 测试转弯角度计算逻辑，验证90度转弯是否正确
 */
void Test_Turn_Angle_Calculation(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Turn Calc", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Debug", 16);
    delay_ms(2000);
    
    while(1) {
        // 模拟在不同yaw角度下的转弯计算
        float current_yaw = yaw;
        float target_yaw = current_yaw + 90.0f;
        
        // 归一化目标角度
        while (target_yaw > 180.0f) target_yaw -= 360.0f;
        while (target_yaw < -180.0f) target_yaw += 360.0f;
        
        // 计算实际转弯角度差
        float turn_angle = target_yaw - current_yaw;
        if (turn_angle > 180.0f) turn_angle -= 360.0f;
        if (turn_angle < -180.0f) turn_angle += 360.0f;
        
        OLED_Clear();
        
        // 显示当前角度
        sprintf(oled_buffer, "Cur:%.1f", current_yaw);
        OLED_ShowString(0, 0, (uint8_t*)oled_buffer, 16);
        
        // 显示目标角度
        sprintf(oled_buffer, "Tgt:%.1f", target_yaw);
        OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
        
        // 显示转弯角度差
        sprintf(oled_buffer, "Turn:%.1f", turn_angle);
        OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
        
        // 检查转弯角度是否正确
        if (fabs(turn_angle - 90.0f) < 1.0f) {
            OLED_ShowString(0, 6, (uint8_t*)"CORRECT", 16);
        } else if (fabs(turn_angle + 270.0f) < 1.0f || fabs(turn_angle - 270.0f) < 1.0f) {
            OLED_ShowString(0, 6, (uint8_t*)"ERROR:270", 16);
        } else if (fabs(turn_angle + 90.0f) < 1.0f) {
            OLED_ShowString(0, 6, (uint8_t*)"ERROR:-90", 16);
        } else {
            OLED_ShowString(0, 6, (uint8_t*)"ERROR:???", 16);
        }
        
        // 显示象限信息
        if (current_yaw >= 0 && current_yaw <= 90) {
            OLED_ShowString(64, 0, (uint8_t*)"Q1", 16);
        } else if (current_yaw > 90 && current_yaw <= 180) {
            OLED_ShowString(64, 0, (uint8_t*)"Q2", 16);
        } else if (current_yaw >= -180 && current_yaw < -90) {
            OLED_ShowString(64, 0, (uint8_t*)"Q3", 16);
        } else if (current_yaw >= -90 && current_yaw < 0) {
            OLED_ShowString(64, 0, (uint8_t*)"Q4", 16);
        }
        
        // 显示目标象限
        if (target_yaw >= 0 && target_yaw <= 90) {
            OLED_ShowString(64, 2, (uint8_t*)"->Q1", 16);
        } else if (target_yaw > 90 && target_yaw <= 180) {
            OLED_ShowString(64, 2, (uint8_t*)"->Q2", 16);
        } else if (target_yaw >= -180 && target_yaw < -90) {
            OLED_ShowString(64, 2, (uint8_t*)"->Q3", 16);
        } else if (target_yaw >= -90 && target_yaw < 0) {
            OLED_ShowString(64, 2, (uint8_t*)"->Q4", 16);
        }
        
        // 显示方向
        if (turn_angle > 0) {
            OLED_ShowString(64, 4, (uint8_t*)"CCW", 16);
        } else if (turn_angle < 0) {
            OLED_ShowString(64, 4, (uint8_t*)"CW", 16);
        } else {
            OLED_ShowString(64, 4, (uint8_t*)"ZERO", 16);
        }
        
        delay_ms(200);
    }
}

/**
 * @brief 测试MPU6050角度跨越特性，专门观察180度边界
 */
void Test_Angle_Crossing_Debug(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Angle Cross", 16);
    OLED_ShowString(0, 2, (uint8_t*)"Debug Mode", 16);
    delay_ms(2000);
    
    float prev_yaw = yaw;  // 记录上一次的yaw值
    float total_rotation = 0.0f;  // 累积总旋转角度
    uint32_t cross_count = 0;  // 角度跨越次数
    
    while(1) {
        // 计算角度变化
        float yaw_delta = yaw - prev_yaw;
        
        // 检测角度跨越（大于180度的突变）
        bool crossed_boundary = false;
        if (yaw_delta > 180.0f) {
            // 从-180跳到+180（逆时针跨越）
            yaw_delta -= 360.0f;
            crossed_boundary = true;
            cross_count++;
        } else if (yaw_delta < -180.0f) {
            // 从+180跳到-180（顺时针跨越）
            yaw_delta += 360.0f;
            crossed_boundary = true;
            cross_count++;
        }
        
        // 累积总旋转角度
        total_rotation += yaw_delta;
        
        OLED_Clear();
        
        // 显示当前yaw角
        sprintf(oled_buffer, "Yaw:%.2f", yaw);
        OLED_ShowString(0, 0, (uint8_t*)oled_buffer, 16);
        
        // 显示上一次yaw角
        sprintf(oled_buffer, "Prev:%.2f", prev_yaw);
        OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
        
        // 显示角度变化
        sprintf(oled_buffer, "Delta:%.2f", yaw_delta);
        OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
        
        // 显示累积旋转角度
        sprintf(oled_buffer, "Total:%.1f", total_rotation);
        OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
        
        // 显示角度跨越检测
        if (crossed_boundary) {
            OLED_ShowString(64, 0, (uint8_t*)"CROSSED!", 16);
        } else {
            OLED_ShowString(64, 0, (uint8_t*)"Normal", 16);
        }
        
        // 显示跨越次数
        sprintf(oled_buffer, "Cross:%d", (int)cross_count);
        OLED_ShowString(64, 2, (uint8_t*)oled_buffer, 16);
        
        // 显示角度所在象限
        if (yaw >= 0 && yaw <= 90) {
            OLED_ShowString(64, 4, (uint8_t*)"Q1:0-90", 16);
        } else if (yaw > 90 && yaw <= 180) {
            OLED_ShowString(64, 4, (uint8_t*)"Q2:90-180", 16);
        } else if (yaw >= -180 && yaw < -90) {
            OLED_ShowString(64, 4, (uint8_t*)"Q3:-180--90", 16);
        } else if (yaw >= -90 && yaw < 0) {
            OLED_ShowString(64, 4, (uint8_t*)"Q4:-90-0", 16);
        }
        
        // 显示90度倍数检测
        float nearest_90 = round(yaw / 90.0f) * 90.0f;
        if (nearest_90 > 180.0f) nearest_90 -= 360.0f;
        if (nearest_90 < -180.0f) nearest_90 += 360.0f;
        
        if (fabs(yaw - nearest_90) < 5.0f) {
            sprintf(oled_buffer, "Near:%.0f", nearest_90);
            OLED_ShowString(64, 6, (uint8_t*)oled_buffer, 16);
        } else {
            OLED_ShowString(64, 6, (uint8_t*)"Far:90x", 16);
        }
        
        // 更新上一次yaw值
        prev_yaw = yaw;
        
        delay_ms(100); // 更新频率10Hz
    }
}

/**
 * @brief 显示MPU6050的yaw角原始信息，用于调试陀螺仪数据
 */
void Test_Display_Yaw_Info(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, (uint8_t*)"Yaw Debug", 16);
    OLED_ShowString(0, 2, (uint8_t*)"MPU6050 Data", 16);
    delay_ms(2000);
    
    float initial_yaw = yaw;  // 记录初始角度作为参考
    uint32_t start_time = tick_ms;
    
    while(1) {
        // 计算相对于初始角度的变化
        float yaw_diff = yaw - initial_yaw;
        
        // 处理角度跨越±180度的情况
        if (yaw_diff > 180.0f) yaw_diff -= 360.0f;
        if (yaw_diff < -180.0f) yaw_diff += 360.0f;
        
        // 计算运行时间
        uint32_t elapsed_time = (tick_ms - start_time) / 1000; // 转换为秒
        
        OLED_Clear();
        
        // 显示当前yaw角（原始值）
        sprintf(oled_buffer, "Yaw:%.2f", yaw);
        OLED_ShowString(0, 0, (uint8_t*)oled_buffer, 16);
        
        // 显示初始yaw角
        sprintf(oled_buffer, "Init:%.2f", initial_yaw);
        OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
        
        // 显示角度变化
        sprintf(oled_buffer, "Diff:%.2f", yaw_diff);
        OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
        
        // 显示运行时间
        sprintf(oled_buffer, "Time:%ds", (int)elapsed_time);
        OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
        
        // 显示角度状态指示
        if (fabs(yaw_diff) < 2.0f) {
            OLED_ShowString(64, 0, (uint8_t*)"Stable", 16);
        } else if (yaw_diff > 0) {
            OLED_ShowString(64, 0, (uint8_t*)"CW", 16);
        } else {
            OLED_ShowString(64, 0, (uint8_t*)"CCW", 16);
        }
        
        // 显示角度范围检查
        if (yaw >= -180.0f && yaw <= 180.0f) {
            OLED_ShowString(64, 2, (uint8_t*)"RNG:OK", 16);
        } else {
            OLED_ShowString(64, 2, (uint8_t*)"RNG:ERR", 16);
        }
        
        // 显示精确到小数点后3位的yaw值（用于精密调试）
        sprintf(oled_buffer, "Raw:%.3f", yaw);
        OLED_ShowString(64, 4, (uint8_t*)oled_buffer, 16);
        
        // 显示是否接近特殊角度（0, ±90, ±180度）
        float special_angles[] = {0.0f, 90.0f, -90.0f, 180.0f, -180.0f};
        const char* special_names[] = {"0", "90", "-90", "180", "-180"};
        bool near_special = false;
        
        for (int i = 0; i < 5; i++) {
            if (fabs(yaw - special_angles[i]) < 5.0f) {
                sprintf(oled_buffer, "Nr:%s", special_names[i]);
                OLED_ShowString(64, 6, (uint8_t*)oled_buffer, 16);
                near_special = true;
                break;
            }
        }
        
        if (!near_special) {
            OLED_ShowString(64, 6, (uint8_t*)"Nr:None", 16);
        }
        
        delay_ms(100); // 更新频率10Hz
    }
}


// /**
//  * @brief 检查循迹传感器状态，用于调试传感器安装是否正确
//  */
// void Test_Check_Line_Sensors(void)
// {
//     char oled_buffer[40];
//     OLED_Clear();
//     OLED_ShowString(0, 0, (uint8_t*)"Sensor Test", 16);
//     OLED_ShowString(0, 2, (uint8_t*)"Check if", 16);
//     OLED_ShowString(0, 4, (uint8_t*)"inverted", 16);
//     delay_ms(2000);
    
//     while(1) {
//         LineTracker_ReadSensors();
        
//         // 显示传感器原始值
//         sprintf(oled_buffer, "S:%d%d%d%d%d%d%d",
//                 g_lineTracker.sensorValue[0], g_lineTracker.sensorValue[1],
//                 g_lineTracker.sensorValue[2], g_lineTracker.sensorValue[3],
//                 g_lineTracker.sensorValue[4], g_lineTracker.sensorValue[5],
//                 g_lineTracker.sensorValue[6]);
//         OLED_ShowString(0, 0, (uint8_t*)oled_buffer, 16);
        
//         // 显示线位置
//         sprintf(oled_buffer, "Pos:%-4d", g_lineTracker.linePosition);
//         OLED_ShowString(0, 2, (uint8_t*)oled_buffer, 16);
        
//         // 显示状态
//         const char* state_str = "Unk";
//         switch(g_lineTracker.lineState) {
//             case LINE_STATE_ON_LINE: state_str = "OnLine"; break;
//             case LINE_STATE_LEFT_TURN: state_str = "Left"; break;
//             case LINE_STATE_RIGHT_TURN: state_str = "Right"; break;
//             case LINE_STATE_NO_LINE: state_str = "NoLine"; break;
//             case LINE_STATE_ALL_LINE: state_str = "All"; break;
//             default: break;
//         }
//         sprintf(oled_buffer, "ST: %s", state_str);
//         OLED_ShowString(0, 4, (uint8_t*)oled_buffer, 16);
        
//         // 显示位图
//         sprintf(oled_buffer, "Bits:0x%02X", g_lineTracker.sensorBits);
//         OLED_ShowString(0, 6, (uint8_t*)oled_buffer, 16);
        
//         delay_ms(100);
//     }
// }
