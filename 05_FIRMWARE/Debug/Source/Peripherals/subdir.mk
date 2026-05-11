################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/Peripherals/DFPLAYER.c \
../Source/Peripherals/ssd1306.c \
../Source/Peripherals/ssd1306_fonts.c \
../Source/Peripherals/ui_assets.c 

OBJS += \
./Source/Peripherals/DFPLAYER.o \
./Source/Peripherals/ssd1306.o \
./Source/Peripherals/ssd1306_fonts.o \
./Source/Peripherals/ui_assets.o 

C_DEPS += \
./Source/Peripherals/DFPLAYER.d \
./Source/Peripherals/ssd1306.d \
./Source/Peripherals/ssd1306_fonts.d \
./Source/Peripherals/ui_assets.d 


# Each subdirectory must supply rules for building sources it contributes
Source/Peripherals/%.o Source/Peripherals/%.su Source/Peripherals/%.cyclo: ../Source/Peripherals/%.c Source/Peripherals/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/STM32/Workspace/api_1/Source/App" -I"E:/STM32/Workspace/api_1/Source/BSP" -I"E:/STM32/Workspace/api_1/Source/Peripherals" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Source-2f-Peripherals

clean-Source-2f-Peripherals:
	-$(RM) ./Source/Peripherals/DFPLAYER.cyclo ./Source/Peripherals/DFPLAYER.d ./Source/Peripherals/DFPLAYER.o ./Source/Peripherals/DFPLAYER.su ./Source/Peripherals/ssd1306.cyclo ./Source/Peripherals/ssd1306.d ./Source/Peripherals/ssd1306.o ./Source/Peripherals/ssd1306.su ./Source/Peripherals/ssd1306_fonts.cyclo ./Source/Peripherals/ssd1306_fonts.d ./Source/Peripherals/ssd1306_fonts.o ./Source/Peripherals/ssd1306_fonts.su ./Source/Peripherals/ui_assets.cyclo ./Source/Peripherals/ui_assets.d ./Source/Peripherals/ui_assets.o ./Source/Peripherals/ui_assets.su

.PHONY: clean-Source-2f-Peripherals

