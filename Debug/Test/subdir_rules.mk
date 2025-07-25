################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Test/%.o: ../Test/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/LSM6DSV16X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/VL53L0X" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/LineTracker" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Test" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Motor_Encoder_PID" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/WIT" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/BNO08X_UART_RVC" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_GPIO" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/Ultrasonic_Capture" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Hardware_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_I2C" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/OLED_Software_SPI" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MPU6050" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Debug" -I"C:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/AkiRin/workspace_ccstheia/mspm0-modules-car/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"Test/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


