/*
 * test.h - 电机测试模块
 * 
 * 用于电机性能测试和数据显示
 */

#ifndef TEST_H_
#define TEST_H_

#include <stdint.h>

// 电机测试相关变量结构体
typedef struct {
    int32_t left_encoder_count;    // 左电机编码器累计计数
    float left_current_speed;      // 左电机当前速度(pps)
    int32_t right_encoder_count;   // 右电机编码器累计计数
    float right_current_speed;     // 右电机当前速度(pps)
    unsigned long last_update_time; // 上次更新时间
} MotorTest_t;

// 全局测试数据实例
extern MotorTest_t motor_test;

// 函数声明
void Test_Init(void);                    // 测试模块初始化
void Test_DualMotorStart(void);          // 开始双电机测试
void Test_DualMotorLoop(void);           // 双电机测试主循环
void Test_DisplayEncoderData(void);     // 显示编码器数据

// 循迹测试函数
void Test_LineTrackerInit(void);        // 循迹测试初始化
void Test_LineTrackerLoop(void);        // 循迹测试主循环
void Test_LineTrackerDisplay(void);     // 循迹状态显示

#endif /* TEST_H_ */
