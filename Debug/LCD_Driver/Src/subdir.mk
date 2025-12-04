################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LCD_Driver/Src/lcd_driver.c 

OBJS += \
./LCD_Driver/Src/lcd_driver.o 

C_DEPS += \
./LCD_Driver/Src/lcd_driver.d 


# Each subdirectory must supply rules for building sources it contributes
LCD_Driver/Src/%.o LCD_Driver/Src/%.su LCD_Driver/Src/%.cyclo: ../LCD_Driver/Src/%.c LCD_Driver/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/LM75B" -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/LCD_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LCD_Driver-2f-Src

clean-LCD_Driver-2f-Src:
	-$(RM) ./LCD_Driver/Src/lcd_driver.cyclo ./LCD_Driver/Src/lcd_driver.d ./LCD_Driver/Src/lcd_driver.o ./LCD_Driver/Src/lcd_driver.su

.PHONY: clean-LCD_Driver-2f-Src

