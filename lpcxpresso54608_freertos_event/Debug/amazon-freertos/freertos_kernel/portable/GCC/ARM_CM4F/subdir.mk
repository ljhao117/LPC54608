################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.c 

OBJS += \
./amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.o 

C_DEPS += \
./amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F/port.d 


# Each subdirectory must supply rules for building sources it contributes
amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F/%.o: ../amazon-freertos/freertos_kernel/portable/GCC/ARM_CM4F/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_LPC54608J512ET180=1 -DCPU_LPC54608J512ET180_cm4 -DCPU_LPC54608 -D__USE_CMSIS -DSERIAL_PORT_TYPE_UART=1 -DFSL_RTOS_FREE_RTOS -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -DDEBUG -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\board" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\source" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\drivers" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\device" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\CMSIS" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\amazon-freertos\freertos_kernel\include" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\amazon-freertos\freertos_kernel\portable\GCC\ARM_CM4F" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\utilities" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\component\serial_manager" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\component\lists" -I"D:\Gitspace\lpcxpresso54608\lpcxpresso54608_freertos_event\component\uart" -O0 -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


