/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000



/* Defines for PWM_MOTOR */
#define PWM_MOTOR_INST                                                     TIMA0
#define PWM_MOTOR_INST_IRQHandler                               TIMA0_IRQHandler
#define PWM_MOTOR_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define PWM_MOTOR_INST_CLK_FREQ                                         80000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_MOTOR_C0_PORT                                             GPIOB
#define GPIO_PWM_MOTOR_C0_PIN                                     DL_GPIO_PIN_14
#define GPIO_PWM_MOTOR_C0_IOMUX                                  (IOMUX_PINCM31)
#define GPIO_PWM_MOTOR_C0_IOMUX_FUNC                 IOMUX_PINCM31_PF_TIMA0_CCP0
#define GPIO_PWM_MOTOR_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_MOTOR_C1_PORT                                             GPIOA
#define GPIO_PWM_MOTOR_C1_PIN                                      DL_GPIO_PIN_7
#define GPIO_PWM_MOTOR_C1_IOMUX                                  (IOMUX_PINCM14)
#define GPIO_PWM_MOTOR_C1_IOMUX_FUNC                 IOMUX_PINCM14_PF_TIMA0_CCP1
#define GPIO_PWM_MOTOR_C1_IDX                                DL_TIMER_CC_1_INDEX



/* Defines for TIMER_CALC */
#define TIMER_CALC_INST                                                  (TIMA1)
#define TIMER_CALC_INST_IRQHandler                              TIMA1_IRQHandler
#define TIMER_CALC_INST_INT_IRQN                                (TIMA1_INT_IRQn)
#define TIMER_CALC_INST_LOAD_VALUE                                       (3124U)




/* Defines for I2C_MPU6050 */
#define I2C_MPU6050_INST                                                    I2C1
#define I2C_MPU6050_INST_IRQHandler                              I2C1_IRQHandler
#define I2C_MPU6050_INST_INT_IRQN                                  I2C1_INT_IRQn
#define I2C_MPU6050_BUS_SPEED_HZ                                          400000
#define GPIO_I2C_MPU6050_SDA_PORT                                          GPIOB
#define GPIO_I2C_MPU6050_SDA_PIN                                   DL_GPIO_PIN_3
#define GPIO_I2C_MPU6050_IOMUX_SDA                               (IOMUX_PINCM16)
#define GPIO_I2C_MPU6050_IOMUX_SDA_FUNC                IOMUX_PINCM16_PF_I2C1_SDA
#define GPIO_I2C_MPU6050_SCL_PORT                                          GPIOB
#define GPIO_I2C_MPU6050_SCL_PIN                                   DL_GPIO_PIN_2
#define GPIO_I2C_MPU6050_IOMUX_SCL                               (IOMUX_PINCM15)
#define GPIO_I2C_MPU6050_IOMUX_SCL_FUNC                IOMUX_PINCM15_PF_I2C1_SCL

/* Defines for I2C_OLED */
#define I2C_OLED_INST                                                       I2C0
#define I2C_OLED_INST_IRQHandler                                 I2C0_IRQHandler
#define I2C_OLED_INST_INT_IRQN                                     I2C0_INT_IRQn
#define I2C_OLED_BUS_SPEED_HZ                                             400000
#define GPIO_I2C_OLED_SDA_PORT                                             GPIOA
#define GPIO_I2C_OLED_SDA_PIN                                      DL_GPIO_PIN_0
#define GPIO_I2C_OLED_IOMUX_SDA                                   (IOMUX_PINCM1)
#define GPIO_I2C_OLED_IOMUX_SDA_FUNC                    IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_I2C_OLED_SCL_PORT                                             GPIOA
#define GPIO_I2C_OLED_SCL_PIN                                      DL_GPIO_PIN_1
#define GPIO_I2C_OLED_IOMUX_SCL                                   (IOMUX_PINCM2)
#define GPIO_I2C_OLED_IOMUX_SCL_FUNC                    IOMUX_PINCM2_PF_I2C0_SCL



/* Port definition for Pin Group GPIO_MPU6050 */
#define GPIO_MPU6050_PORT                                                (GPIOB)

/* Defines for PIN_INT: GPIOB.1 with pinCMx 13 on package pin 48 */
// groups represented: ["GPIO_ENCODER","GPIO_MPU6050"]
// pins affected: ["PIN_A1","PIN_A2","PIN_B1","PIN_B2","PIN_INT"]
#define GPIO_MULTIPLE_GPIOB_INT_IRQN                            (GPIOB_INT_IRQn)
#define GPIO_MULTIPLE_GPIOB_INT_IIDX            (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define GPIO_MPU6050_PIN_INT_IIDX                            (DL_GPIO_IIDX_DIO1)
#define GPIO_MPU6050_PIN_INT_PIN                                 (DL_GPIO_PIN_1)
#define GPIO_MPU6050_PIN_INT_IOMUX                               (IOMUX_PINCM13)
/* Port definition for Pin Group GPIO_MOTOR */
#define GPIO_MOTOR_PORT                                                  (GPIOB)

/* Defines for PIN_AIN1: GPIOB.9 with pinCMx 26 on package pin 61 */
#define GPIO_MOTOR_PIN_AIN1_PIN                                  (DL_GPIO_PIN_9)
#define GPIO_MOTOR_PIN_AIN1_IOMUX                                (IOMUX_PINCM26)
/* Defines for PIN_AIN2: GPIOB.10 with pinCMx 27 on package pin 62 */
#define GPIO_MOTOR_PIN_AIN2_PIN                                 (DL_GPIO_PIN_10)
#define GPIO_MOTOR_PIN_AIN2_IOMUX                                (IOMUX_PINCM27)
/* Defines for PIN_BIN1: GPIOB.7 with pinCMx 24 on package pin 59 */
#define GPIO_MOTOR_PIN_BIN1_PIN                                  (DL_GPIO_PIN_7)
#define GPIO_MOTOR_PIN_BIN1_IOMUX                                (IOMUX_PINCM24)
/* Defines for PIN_BIN2: GPIOB.6 with pinCMx 23 on package pin 58 */
#define GPIO_MOTOR_PIN_BIN2_PIN                                  (DL_GPIO_PIN_6)
#define GPIO_MOTOR_PIN_BIN2_IOMUX                                (IOMUX_PINCM23)
/* Port definition for Pin Group GPIO_ENCODER */
#define GPIO_ENCODER_PORT                                                (GPIOB)

/* Defines for PIN_A1: GPIOB.11 with pinCMx 28 on package pin 63 */
#define GPIO_ENCODER_PIN_A1_IIDX                            (DL_GPIO_IIDX_DIO11)
#define GPIO_ENCODER_PIN_A1_PIN                                 (DL_GPIO_PIN_11)
#define GPIO_ENCODER_PIN_A1_IOMUX                                (IOMUX_PINCM28)
/* Defines for PIN_A2: GPIOB.4 with pinCMx 17 on package pin 52 */
#define GPIO_ENCODER_PIN_A2_IIDX                             (DL_GPIO_IIDX_DIO4)
#define GPIO_ENCODER_PIN_A2_PIN                                  (DL_GPIO_PIN_4)
#define GPIO_ENCODER_PIN_A2_IOMUX                                (IOMUX_PINCM17)
/* Defines for PIN_B1: GPIOB.12 with pinCMx 29 on package pin 64 */
#define GPIO_ENCODER_PIN_B1_IIDX                            (DL_GPIO_IIDX_DIO12)
#define GPIO_ENCODER_PIN_B1_PIN                                 (DL_GPIO_PIN_12)
#define GPIO_ENCODER_PIN_B1_IOMUX                                (IOMUX_PINCM29)
/* Defines for PIN_B2: GPIOB.5 with pinCMx 18 on package pin 53 */
#define GPIO_ENCODER_PIN_B2_IIDX                             (DL_GPIO_IIDX_DIO5)
#define GPIO_ENCODER_PIN_B2_PIN                                  (DL_GPIO_PIN_5)
#define GPIO_ENCODER_PIN_B2_IOMUX                                (IOMUX_PINCM18)
/* Defines for PIN_OUT1: GPIOB.19 with pinCMx 45 on package pin 16 */
#define GPIO_TRM_PIN_OUT1_PORT                                           (GPIOB)
#define GPIO_TRM_PIN_OUT1_PIN                                   (DL_GPIO_PIN_19)
#define GPIO_TRM_PIN_OUT1_IOMUX                                  (IOMUX_PINCM45)
/* Defines for PIN_OUT2: GPIOB.17 with pinCMx 43 on package pin 14 */
#define GPIO_TRM_PIN_OUT2_PORT                                           (GPIOB)
#define GPIO_TRM_PIN_OUT2_PIN                                   (DL_GPIO_PIN_17)
#define GPIO_TRM_PIN_OUT2_IOMUX                                  (IOMUX_PINCM43)
/* Defines for PIN_OUT3: GPIOA.16 with pinCMx 38 on package pin 9 */
#define GPIO_TRM_PIN_OUT3_PORT                                           (GPIOA)
#define GPIO_TRM_PIN_OUT3_PIN                                   (DL_GPIO_PIN_16)
#define GPIO_TRM_PIN_OUT3_IOMUX                                  (IOMUX_PINCM38)
/* Defines for PIN_OUT4: GPIOA.14 with pinCMx 36 on package pin 7 */
#define GPIO_TRM_PIN_OUT4_PORT                                           (GPIOA)
#define GPIO_TRM_PIN_OUT4_PIN                                   (DL_GPIO_PIN_14)
#define GPIO_TRM_PIN_OUT4_IOMUX                                  (IOMUX_PINCM36)
/* Defines for PIN_OUT5: GPIOB.20 with pinCMx 48 on package pin 19 */
#define GPIO_TRM_PIN_OUT5_PORT                                           (GPIOB)
#define GPIO_TRM_PIN_OUT5_PIN                                   (DL_GPIO_PIN_20)
#define GPIO_TRM_PIN_OUT5_IOMUX                                  (IOMUX_PINCM48)
/* Defines for PIN_OUT6: GPIOB.25 with pinCMx 56 on package pin 27 */
#define GPIO_TRM_PIN_OUT6_PORT                                           (GPIOB)
#define GPIO_TRM_PIN_OUT6_PIN                                   (DL_GPIO_PIN_25)
#define GPIO_TRM_PIN_OUT6_IOMUX                                  (IOMUX_PINCM56)
/* Defines for PIN_OUT7: GPIOA.25 with pinCMx 55 on package pin 26 */
#define GPIO_TRM_PIN_OUT7_PORT                                           (GPIOA)
#define GPIO_TRM_PIN_OUT7_PIN                                   (DL_GPIO_PIN_25)
#define GPIO_TRM_PIN_OUT7_IOMUX                                  (IOMUX_PINCM55)
/* Defines for PIN_1: GPIOA.15 with pinCMx 37 on package pin 8 */
#define GPIO_LED_PIN_1_PORT                                              (GPIOA)
#define GPIO_LED_PIN_1_PIN                                      (DL_GPIO_PIN_15)
#define GPIO_LED_PIN_1_IOMUX                                     (IOMUX_PINCM37)
/* Defines for PIN_2: GPIOA.22 with pinCMx 47 on package pin 18 */
#define GPIO_LED_PIN_2_PORT                                              (GPIOA)
#define GPIO_LED_PIN_2_PIN                                      (DL_GPIO_PIN_22)
#define GPIO_LED_PIN_2_IOMUX                                     (IOMUX_PINCM47)
/* Defines for PIN_BEEP: GPIOB.27 with pinCMx 58 on package pin 29 */
#define GPIO_LED_PIN_BEEP_PORT                                           (GPIOB)
#define GPIO_LED_PIN_BEEP_PIN                                   (DL_GPIO_PIN_27)
#define GPIO_LED_PIN_BEEP_IOMUX                                  (IOMUX_PINCM58)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_PWM_MOTOR_init(void);
void SYSCFG_DL_TIMER_CALC_init(void);
void SYSCFG_DL_I2C_MPU6050_init(void);
void SYSCFG_DL_I2C_OLED_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
