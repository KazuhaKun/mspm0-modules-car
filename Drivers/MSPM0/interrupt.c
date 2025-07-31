#include "ti_msp_dl_config.h"
#include "interrupt.h"
#include "clock.h"
#include "mpu6050.h"
#include "bno08x_uart_rvc.h"
#include "wit.h"
#include "vl53l0x.h"
#include "lsm6dsv16x.h"
#include "motor_control.h"
#include "turn_detection.h"
#include "Encoder.h"

// 函数声明
void Encoder_IRQHandler(void);
void Encoder_Timer_IRQHandler(void);


void SysTick_Handler(void)
{
    tick_ms++;
}

void TIMG8_IRQHandler(void)
{
    // 清除定时器中断标志
    DL_TimerG_clearInterruptStatus(TIMER_TRACKER_INST, DL_TIMER_IIDX_ZERO);
    
    // 更新转弯检测状态
    TurnDetection_Update();
}

#if defined UART_BNO08X_INST_IRQHandler
void UART_BNO08X_INST_IRQHandler(void)
{
    uint8_t checkSum = 0;
    extern uint8_t bno08x_dmaBuffer[19];

    DL_DMA_disableChannel(DMA, DMA_BNO08X_CHAN_ID);
    uint8_t rxSize = 18 - DL_DMA_getTransferSize(DMA, DMA_BNO08X_CHAN_ID);

    if(DL_UART_isRXFIFOEmpty(UART_BNO08X_INST) == false)
        bno08x_dmaBuffer[rxSize++] = DL_UART_receiveData(UART_BNO08X_INST);

    for(int i=2; i<=14; i++)
        checkSum += bno08x_dmaBuffer[i];

    if((rxSize == 19) && (bno08x_dmaBuffer[0] == 0xAA) && (bno08x_dmaBuffer[1] == 0xAA) && (checkSum == bno08x_dmaBuffer[18]))
    {
        bno08x_data.index = bno08x_dmaBuffer[2];
        bno08x_data.yaw = (int16_t)((bno08x_dmaBuffer[4]<<8)|bno08x_dmaBuffer[3]) / 100.0;
        bno08x_data.pitch = (int16_t)((bno08x_dmaBuffer[6]<<8)|bno08x_dmaBuffer[5]) / 100.0;
        bno08x_data.roll = (int16_t)((bno08x_dmaBuffer[8]<<8)|bno08x_dmaBuffer[7]) / 100.0;
        bno08x_data.ax = (bno08x_dmaBuffer[10]<<8)|bno08x_dmaBuffer[9];
        bno08x_data.ay = (bno08x_dmaBuffer[12]<<8)|bno08x_dmaBuffer[11];
        bno08x_data.az = (bno08x_dmaBuffer[14]<<8)|bno08x_dmaBuffer[13];
    }
    
    uint8_t dummy[4];
    DL_UART_drainRXFIFO(UART_BNO08X_INST, dummy, 4);

    DL_DMA_setDestAddr(DMA, DMA_BNO08X_CHAN_ID, (uint32_t) &bno08x_dmaBuffer[0]);
    DL_DMA_setTransferSize(DMA, DMA_BNO08X_CHAN_ID, 18);
    DL_DMA_enableChannel(DMA, DMA_BNO08X_CHAN_ID);
}
#endif

#if defined UART_WIT_INST_IRQHandler
void UART_WIT_INST_IRQHandler(void)
{
    uint8_t checkSum, packCnt = 0;
    extern uint8_t wit_dmaBuffer[33];

    DL_DMA_disableChannel(DMA, DMA_WIT_CHAN_ID);
    uint8_t rxSize = 32 - DL_DMA_getTransferSize(DMA, DMA_WIT_CHAN_ID);

    if(DL_UART_isRXFIFOEmpty(UART_WIT_INST) == false)
        wit_dmaBuffer[rxSize++] = DL_UART_receiveData(UART_WIT_INST);

    while(rxSize >= 11)
    {
        checkSum=0;
        for(int i=packCnt*11; i<(packCnt+1)*11-1; i++)
            checkSum += wit_dmaBuffer[i];

        if((wit_dmaBuffer[packCnt*11] == 0x55) && (checkSum == wit_dmaBuffer[packCnt*11+10]))
        {
            if(wit_dmaBuffer[packCnt*11+1] == 0x51)
            {
                wit_data.ax = (int16_t)((wit_dmaBuffer[packCnt*11+3]<<8)|wit_dmaBuffer[packCnt*11+2]) / 2.048; //mg
                wit_data.ay = (int16_t)((wit_dmaBuffer[packCnt*11+5]<<8)|wit_dmaBuffer[packCnt*11+4]) / 2.048; //mg
                wit_data.az = (int16_t)((wit_dmaBuffer[packCnt*11+7]<<8)|wit_dmaBuffer[packCnt*11+6]) / 2.048; //mg
                wit_data.temperature =  (int16_t)((wit_dmaBuffer[packCnt*11+9]<<8)|wit_dmaBuffer[packCnt*11+8]) / 100.0; //°C
            }
            else if(wit_dmaBuffer[packCnt*11+1] == 0x52)
            {
                wit_data.gx = (int16_t)((wit_dmaBuffer[packCnt*11+3]<<8)|wit_dmaBuffer[packCnt*11+2]) / 16.384; //°/S
                wit_data.gy = (int16_t)((wit_dmaBuffer[packCnt*11+5]<<8)|wit_dmaBuffer[packCnt*11+4]) / 16.384; //°/S
                wit_data.gz = (int16_t)((wit_dmaBuffer[packCnt*11+7]<<8)|wit_dmaBuffer[packCnt*11+6]) / 16.384; //°/S
            }
            else if(wit_dmaBuffer[packCnt*11+1] == 0x53)
            {
                wit_data.roll  = (int16_t)((wit_dmaBuffer[packCnt*11+3]<<8)|wit_dmaBuffer[packCnt*11+2]) / 32768.0 * 180.0; //°
                wit_data.pitch = (int16_t)((wit_dmaBuffer[packCnt*11+5]<<8)|wit_dmaBuffer[packCnt*11+4]) / 32768.0 * 180.0; //°
                wit_data.yaw   = (int16_t)((wit_dmaBuffer[packCnt*11+7]<<8)|wit_dmaBuffer[packCnt*11+6]) / 32768.0 * 180.0; //°
                wit_data.version = (int16_t)((wit_dmaBuffer[packCnt*11+9]<<8)|wit_dmaBuffer[packCnt*11+8]);
            }
        }

        rxSize -= 11;
        packCnt++;
    }
    
    uint8_t dummy[4];
    DL_UART_drainRXFIFO(UART_WIT_INST, dummy, 4);

    DL_DMA_setDestAddr(DMA, DMA_WIT_CHAN_ID, (uint32_t) &wit_dmaBuffer[0]);
    DL_DMA_setTransferSize(DMA, DMA_WIT_CHAN_ID, 32);
    DL_DMA_enableChannel(DMA, DMA_WIT_CHAN_ID);
}
#endif

void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1)) {
        /* GPIOB 多功能中断处理 - MPU6050 和 编码器都在这里 */
        #if defined GPIO_MULTIPLE_GPIOB_INT_IIDX
        case GPIO_MULTIPLE_GPIOB_INT_IIDX:
            // 检查是否是MPU6050中断
            #if defined GPIO_MPU6050_PORT && defined GPIO_MPU6050_PIN_INT_PIN
            if (DL_GPIO_getEnabledInterruptStatus(GPIO_MPU6050_PORT, GPIO_MPU6050_PIN_INT_PIN)) {
                Read_Quad();
                DL_GPIO_clearInterruptStatus(GPIO_MPU6050_PORT, GPIO_MPU6050_PIN_INT_PIN);
            }
            #endif
            
            // 检查是否是编码器中断
            #if defined GPIO_ENCODER_PORT
            uint32_t encoder_pins = GPIO_ENCODER_PIN_A1_PIN | GPIO_ENCODER_PIN_A2_PIN | 
                                   GPIO_ENCODER_PIN_B1_PIN | GPIO_ENCODER_PIN_B2_PIN;
            if (DL_GPIO_getEnabledInterruptStatus(GPIO_ENCODER_PORT, encoder_pins)) {
                Encoder_IRQHandler();
                // 注意：Encoder_IRQHandler内部会清除中断标志
            }
            #endif
            break;
        #endif

        // /* VL53L0X INT */
        // #if defined GPIO_VL53L0X_PIN_GPIO1_PORT
        //     #if defined GPIO_VL53L0X_INT_IIDX
        //     case GPIO_VL53L0X_INT_IIDX:
        //     #elif (GPIO_VL53L0X_PIN_GPIO1_PORT == GPIOA) && (defined GPIO_MULTIPLE_GPIOA_INT_IIDX)
        //     case GPIO_MULTIPLE_GPIOA_INT_IIDX:
        //     #elif (GPIO_VL53L0X_PIN_GPIO1_PORT == GPIOB) && (defined GPIO_MULTIPLE_GPIOB_INT_IIDX)
        //     case GPIO_MULTIPLE_GPIOB_INT_IIDX:
        //     #endif
        //         Read_VL53L0X();
        //         break;
        // #endif

        // /* LSM6DSV16X INT */
        // #if defined GPIO_LSM6DSV16X_PORT
        //     #if defined GPIO_LSM6DSV16X_INT_IIDX
        //     case GPIO_LSM6DSV16X_INT_IIDX:
        //     #elif (GPIO_LSM6DSV16X_PORT == GPIOA) && (defined GPIO_MULTIPLE_GPIOA_INT_IIDX)
        //     case GPIO_MULTIPLE_GPIOA_INT_IIDX:
        //     #elif (GPIO_LSM6DSV16X_PORT == GPIOB) && (defined GPIO_MULTIPLE_GPIOB_INT_IIDX)
        //     case GPIO_MULTIPLE_GPIOB_INT_IIDX:
        //     #endif
        //         Read_LSM6DSV16X();
        //         break;
        // #endif
        
        default:
            break;
    }
}

// 定时器中断处理函数（用于速度计算和PID控制）
void TIMA1_IRQHandler(void)
{
    // 立即清除定时器中断标志，确保中断能够被及时清除
    DL_TimerA_clearInterruptStatus(TIMER_CALC_INST, DL_TIMER_IIDX_ZERO);
    
    // 先处理编码器速度计算
    Encoder_Timer_IRQHandler();
    
    // 然后更新PID控制
    MotorControl_Update();
    
    // // 可选：LED状态指示
    // static uint8_t led_state = 0;
    // if (led_state) {
    //     DL_GPIO_clearPins(GPIO_LED_PORT, GPIO_LED_PIN_1_PIN);
    // } else {
    //     DL_GPIO_setPins(GPIO_LED_PORT, GPIO_LED_PIN_1_PIN);
    // }
    // led_state = !led_state;
}
