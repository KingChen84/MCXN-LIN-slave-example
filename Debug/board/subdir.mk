################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/pin_mux.c 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/pin_mux.d 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/pin_mux.o 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c board/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MCXN947VDF -DCPU_MCXN947VDF_cm33 -DCPU_MCXN947VDF_cm33_core0 -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\source" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\utilities" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\drivers" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\device" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\startup" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\component\uart" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\component\lists" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\CMSIS" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\board" -I"D:\MCUXpresso-Project\frdmmcxn947_lpuart_LIN_slave\frdmmcxn947\driver_examples\lpuart\interrupt\cm33_core0" -O0 -fno-common -g3 -gdwarf-4 -mcpu=cortex-m33 -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-board

clean-board:
	-$(RM) ./board/board.d ./board/board.o ./board/clock_config.d ./board/clock_config.o ./board/pin_mux.d ./board/pin_mux.o

.PHONY: clean-board

