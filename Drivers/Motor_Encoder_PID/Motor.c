#include "Motor.h"

// 内部变量
static uint8_t pwm_running = 0;           // PWM运行状态

/**
 * @brief 初始化电机
 */
void Motor_Init(void)
{
    // 停止所有电机
    Motor_Stop(MOTOR_ALL);
    pwm_running = 0;
}

/**
 * @brief 启动PWM
 */
void Motor_PWM_Start(void)
{
    if (!pwm_running) {
        DL_TimerA_startCounter(PWM_MOTOR_INST);
        pwm_running = 1;
    }
}

/**
 * @brief 停止PWM
 */
void Motor_PWM_Stop(void)
{
    if (pwm_running) {
        DL_TimerA_stopCounter(PWM_MOTOR_INST);
        pwm_running = 0;
    }
}

/**
 * @brief 检查电机是否运行
 * @return 1=运行, 0=停止
 */
uint8_t Motor_IsRunning(void)
{
    return pwm_running;
}

/**
 * @brief 设置电机方向
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param direction 1=正转, -1=反转, 0=停止
 */
void Motor_SetDirection(uint8_t motor_id, int8_t direction)
{
    if (motor_id == MOTOR_A) {
        // 电机A控制 (AIN1, AIN2)
        switch (direction) {
            case 1:  // 正转
                DL_GPIO_setPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_AIN1_PIN);
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_AIN2_PIN);
                break;
            case -1: // 反转
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_AIN1_PIN);
                DL_GPIO_setPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_AIN2_PIN);
                break;
            default: // 停止
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_AIN1_PIN);
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_AIN2_PIN);
                break;
        }
    } else if (motor_id == MOTOR_B) {
        // 电机B控制 (BIN1, BIN2)
        switch (direction) {
            case 1:  // 正转
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_BIN1_PIN);
                DL_GPIO_setPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_BIN2_PIN);
                break;
            case -1: // 反转
                DL_GPIO_setPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_BIN1_PIN);
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_BIN2_PIN);
                break;
            default: // 停止
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_BIN1_PIN);
                DL_GPIO_clearPins(GPIO_MOTOR_PORT, GPIO_MOTOR_PIN_BIN2_PIN);
                break;
        }
    }
}

/**
 * @brief 设置PWM占空比
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param duty 占空比 (0.0-1.0)
 */
void Motor_SetPWM(uint8_t motor_id, float duty)
{
    // 限制占空比范围
    if (duty > 1.0f) duty = 1.0f;
    if (duty < 0.0f) duty = 0.0f;

    // 获取period值，应该与配置文件一致
    uint32_t period = 8000;  // 根据配置文件设置

    // 计算PWM比较值
    uint32_t pwm_value = (uint32_t)(period - period * duty);
    
    // 根据电机ID设置对应的PWM通道
    if (motor_id == MOTOR_A) {
        // 电机A使用PWM通道0
        DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, pwm_value, GPIO_PWM_MOTOR_C0_IDX);
    } else if (motor_id == MOTOR_B) {
        // 电机B使用PWM通道1
        DL_TimerA_setCaptureCompareValue(PWM_MOTOR_INST, pwm_value, GPIO_PWM_MOTOR_C1_IDX);
    }
}

/**
 * @brief 停止电机
 * @param motor_id 电机ID (0=电机A, 1=电机B, 2=全部)
 */
void Motor_Stop(uint8_t motor_id)
{
    if (motor_id == MOTOR_ALL) {
        // 停止所有电机
        Motor_SetDirection(MOTOR_A, 0);
        Motor_SetDirection(MOTOR_B, 0);
        Motor_SetPWM(MOTOR_A, 0);
        Motor_SetPWM(MOTOR_B, 0);
    } else if (motor_id == MOTOR_A || motor_id == MOTOR_B) {
        // 停止指定电机
        Motor_SetDirection(motor_id, 0);
        Motor_SetPWM(motor_id, 0);
    }
}

/**
 * @brief 电机综合控制函数
 * @param motor_id 电机ID (0=电机A, 1=电机B)
 * @param speed 速度 (-1.0到1.0)，负值反转，正值正转，0停止
 */
void Motor_Run(uint8_t motor_id, float speed)
{
    if (speed == 0.0f) {
        Motor_Stop(motor_id);
        return;
    }
    
    // 获取绝对值速度
    float abs_speed = speed > 0 ? speed : -speed;
    
    // 设置最小PWM阈值，确保电机能够启动
    if (abs_speed < MIN_PWM_DUTY && abs_speed > 0.0f) {
        abs_speed = MIN_PWM_DUTY;
    }
    
    // 启动PWM（如果未启动）
    if (!pwm_running) {
        Motor_PWM_Start();
    }

    // 设置方向和速度
    Motor_SetDirection(motor_id, speed > 0 ? 1 : -1);
    Motor_SetPWM(motor_id, abs_speed);
}
