################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LM75B/lm75b.c 

OBJS += \
./LM75B/lm75b.o 

C_DEPS += \
./LM75B/lm75b.d 


# Each subdirectory must supply rules for building sources it contributes
LM75B/%.o LM75B/%.su LM75B/%.cyclo: ../LM75B/%.c LM75B/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/LM75B" -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/LCD_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-LM75B

clean-LM75B:
	-$(RM) ./LM75B/lm75b.cyclo ./LM75B/lm75b.d ./LM75B/lm75b.o ./LM75B/lm75b.su

.PHONY: clean-LM75B

