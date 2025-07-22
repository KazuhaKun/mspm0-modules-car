################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/LSM6DSV16X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/VL53L0X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Motor_Encoder_PID" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/WIT" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/BNO08X_UART_RVC" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_GPIO" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_Capture" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MPU6050" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Debug" -I"C:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-523253039: ../mspm0-modules.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs2020/ccs/utils/sysconfig_1.24.0/sysconfig_cli.bat" --script "C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/mspm0-modules.syscfg" -o "." -s "C:/ti/mspm0_sdk_2_05_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-523253039 ../mspm0-modules.syscfg
device.opt: build-523253039
device.cmd.genlibs: build-523253039
ti_msp_dl_config.c: build-523253039
ti_msp_dl_config.h: build-523253039
Event.dot: build-523253039

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/LSM6DSV16X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/VL53L0X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Motor_Encoder_PID" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/WIT" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/BNO08X_UART_RVC" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_GPIO" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_Capture" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MPU6050" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Debug" -I"C:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: C:/ti/mspm0_sdk_2_05_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/LSM6DSV16X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/VL53L0X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Motor_Encoder_PID" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/WIT" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/BNO08X_UART_RVC" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_GPIO" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_Capture" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MPU6050" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Debug" -I"C:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


