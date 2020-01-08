################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_emc.c \
../drivers/fsl_flexcomm.c \
../drivers/fsl_gint.c \
../drivers/fsl_gpio.c \
../drivers/fsl_inputmux.c \
../drivers/fsl_pint.c \
../drivers/fsl_power.c \
../drivers/fsl_reset.c \
../drivers/fsl_usart.c 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_emc.o \
./drivers/fsl_flexcomm.o \
./drivers/fsl_gint.o \
./drivers/fsl_gpio.o \
./drivers/fsl_inputmux.o \
./drivers/fsl_pint.o \
./drivers/fsl_power.o \
./drivers/fsl_reset.o \
./drivers/fsl_usart.o 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_emc.d \
./drivers/fsl_flexcomm.d \
./drivers/fsl_gint.d \
./drivers/fsl_gpio.d \
./drivers/fsl_inputmux.d \
./drivers/fsl_pint.d \
./drivers/fsl_power.d \
./drivers/fsl_reset.d \
./drivers/fsl_usart.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_LPC54608J512ET180=1 -DCPU_LPC54608J512ET180_cm4 -DCPU_LPC54608 -D__USE_CMSIS -DSERIAL_PORT_TYPE_UART=1 -DFSL_RTOS_FREE_RTOS -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -DDEBUG -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\board" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\source\inc" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\source" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\drivers" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\device" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\CMSIS" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\amazon-freertos\freertos_kernel\include" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\amazon-freertos\freertos_kernel\portable\GCC\ARM_CM4F" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\utilities" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\component\serial_manager" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\component\lists" -I"D:\Gitspace\LPC54608\lpcxpresso54608_freertos_event\component\uart" -O0 -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


