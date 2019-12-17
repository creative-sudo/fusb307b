################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Fusb307b/Src/core.c \
../Fusb307b/Src/display_port.c \
../Fusb307b/Src/dpm.c \
../Fusb307b/Src/hostcomm.c \
../Fusb307b/Src/log.c \
../Fusb307b/Src/observer.c \
../Fusb307b/Src/policy.c \
../Fusb307b/Src/port.c \
../Fusb307b/Src/protocol.c \
../Fusb307b/Src/registers.c \
../Fusb307b/Src/systempolicy.c \
../Fusb307b/Src/timer.c \
../Fusb307b/Src/typec.c \
../Fusb307b/Src/vdm.c \
../Fusb307b/Src/vendor_info.c 

OBJS += \
./Fusb307b/Src/core.o \
./Fusb307b/Src/display_port.o \
./Fusb307b/Src/dpm.o \
./Fusb307b/Src/hostcomm.o \
./Fusb307b/Src/log.o \
./Fusb307b/Src/observer.o \
./Fusb307b/Src/policy.o \
./Fusb307b/Src/port.o \
./Fusb307b/Src/protocol.o \
./Fusb307b/Src/registers.o \
./Fusb307b/Src/systempolicy.o \
./Fusb307b/Src/timer.o \
./Fusb307b/Src/typec.o \
./Fusb307b/Src/vdm.o \
./Fusb307b/Src/vendor_info.o 

C_DEPS += \
./Fusb307b/Src/core.d \
./Fusb307b/Src/display_port.d \
./Fusb307b/Src/dpm.d \
./Fusb307b/Src/hostcomm.d \
./Fusb307b/Src/log.d \
./Fusb307b/Src/observer.d \
./Fusb307b/Src/policy.d \
./Fusb307b/Src/port.d \
./Fusb307b/Src/protocol.d \
./Fusb307b/Src/registers.d \
./Fusb307b/Src/systempolicy.d \
./Fusb307b/Src/timer.d \
./Fusb307b/Src/typec.d \
./Fusb307b/Src/vdm.d \
./Fusb307b/Src/vendor_info.d 


# Each subdirectory must supply rules for building sources it contributes
Fusb307b/Src/core.o: ../Fusb307b/Src/core.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/core.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/display_port.o: ../Fusb307b/Src/display_port.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/display_port.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/dpm.o: ../Fusb307b/Src/dpm.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/dpm.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/hostcomm.o: ../Fusb307b/Src/hostcomm.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/hostcomm.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/log.o: ../Fusb307b/Src/log.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/log.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/observer.o: ../Fusb307b/Src/observer.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/observer.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/policy.o: ../Fusb307b/Src/policy.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/policy.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/port.o: ../Fusb307b/Src/port.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/port.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/protocol.o: ../Fusb307b/Src/protocol.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/protocol.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/registers.o: ../Fusb307b/Src/registers.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/registers.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/systempolicy.o: ../Fusb307b/Src/systempolicy.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/systempolicy.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/timer.o: ../Fusb307b/Src/timer.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/timer.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/typec.o: ../Fusb307b/Src/typec.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/typec.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/vdm.o: ../Fusb307b/Src/vdm.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/vdm.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Fusb307b/Src/vendor_info.o: ../Fusb307b/Src/vendor_info.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DFSC_HAVE_DP -DFSC_HAVE_SNK -DPLATFORM_ARM -DFSC_HAVE_VDM -DSTM32L476xx -DDEBUG -c -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Fusb307b/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Fusb307b/Src/vendor_info.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

