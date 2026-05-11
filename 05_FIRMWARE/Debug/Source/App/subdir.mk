################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/App/app_main.c 

OBJS += \
./Source/App/app_main.o 

C_DEPS += \
./Source/App/app_main.d 


# Each subdirectory must supply rules for building sources it contributes
Source/App/%.o Source/App/%.su Source/App/%.cyclo: ../Source/App/%.c Source/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/STM32/Workspace/api_1/Source/App" -I"E:/STM32/Workspace/api_1/Source/BSP" -I"E:/STM32/Workspace/api_1/Source/Peripherals" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-App

clean-Source-2f-App:
	-$(RM) ./Source/App/app_main.cyclo ./Source/App/app_main.d ./Source/App/app_main.o ./Source/App/app_main.su

.PHONY: clean-Source-2f-App

