################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../digTemp_KY_028/digTemp_KY_028.c 

OBJS += \
./digTemp_KY_028/digTemp_KY_028.o 

C_DEPS += \
./digTemp_KY_028/digTemp_KY_028.d 


# Each subdirectory must supply rules for building sources it contributes
digTemp_KY_028/%.o digTemp_KY_028/%.su digTemp_KY_028/%.cyclo: ../digTemp_KY_028/%.c digTemp_KY_028/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/digTemp_KY_028" -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/LM75B" -I"C:/dev/stm32/workspace_1.19.0/STM32F446RE_Project/LCD_Driver/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-digTemp_KY_028

clean-digTemp_KY_028:
	-$(RM) ./digTemp_KY_028/digTemp_KY_028.cyclo ./digTemp_KY_028/digTemp_KY_028.d ./digTemp_KY_028/digTemp_KY_028.o ./digTemp_KY_028/digTemp_KY_028.su

.PHONY: clean-digTemp_KY_028

