#ifndef __ENCODER_H
#define __ENCODER_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

// 编码器相关函数
void Encoder_Init(void);                           // 初始化编码器
int32_t Encoder_GetCount(uint8_t motor_id);        // 获取编码器计数 (0=左电机, 1=右电机)
int32_t Encoder_GetSpeed(uint8_t motor_id);        // 获取编码器速度(每秒脉冲数)带方向
uint32_t Encoder_GetSpeedAbs(uint8_t motor_id);    // 获取编码器速度绝对值
void Encoder_Reset(uint8_t motor_id);              // 重置编码器计数 (0=左电机, 1=右电机, 2=全部)
void Encoder_IRQHandler(void);                     // 编码器中断处理函数

// 定时器中断处理函数（用于速度计算）
void Encoder_Timer_IRQHandler(void);

#endif
