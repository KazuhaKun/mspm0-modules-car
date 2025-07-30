/*
 * test.h
 *
 *  Created on: 2024年7月25日
 *      Author: AkiRin
 */

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

#include "turn_detection.h"  // 添加转弯检测头文件

// 核心测试函数
// void Test_Check_Line_Sensors(void);              // 传感器状态检查
void Test_Square_Movement_Hybrid(void);          // 混合模式正方形循迹
void Test_Display_Yaw_Info(void);                // 显示MPU6050 yaw角原始信息
void Test_Angle_Crossing_Debug(void);            // 测试MPU6050角度跨越特性
void Test_Turn_Angle_Calculation(void);          // 测试转弯角度计算逻辑


#endif /* TEST_TEST_H_ */