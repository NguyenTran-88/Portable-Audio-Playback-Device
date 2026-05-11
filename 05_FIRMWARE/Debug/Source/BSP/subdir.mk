################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/BSP/bsp_audio.c \
../Source/BSP/bsp_battery.c \
../Source/BSP/bsp_button.c \
../Source/BSP/bsp_display.c 

OBJS += \
./Source/BSP/bsp_audio.o \
./Source/BSP/bsp_battery.o \
./Source/BSP/bsp_button.o \
./Source/BSP/bsp_display.o 

C_DEPS += \
./Source/BSP/bsp_audio.d \
./Source/BSP/bsp_battery.d \
./Source/BSP/bsp_button.d \
./Source/BSP/bsp_display.d 


# Each subdirectory must supply rules for building sources it contributes
Source/BSP/%.o Source/BSP/%.su Source/BSP/%.cyclo: ../Source/BSP/%.c Source/BSP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/STM32/Workspace/api_1/Source/App" -I"E:/STM32/Workspace/api_1/Source/BSP" -I"E:/STM32/Workspace/api_1/Source/Peripherals" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-BSP

clean-Source-2f-BSP:
	-$(RM) ./Source/BSP/bsp_audio.cyclo ./Source/BSP/bsp_audio.d ./Source/BSP/bsp_audio.o ./Source/BSP/bsp_audio.su ./Source/BSP/bsp_battery.cyclo ./Source/BSP/bsp_battery.d ./Source/BSP/bsp_battery.o ./Source/BSP/bsp_battery.su ./Source/BSP/bsp_button.cyclo ./Source/BSP/bsp_button.d ./Source/BSP/bsp_button.o ./Source/BSP/bsp_button.su ./Source/BSP/bsp_display.cyclo ./Source/BSP/bsp_display.d ./Source/BSP/bsp_display.o ./Source/BSP/bsp_display.su

.PHONY: clean-Source-2f-BSP

