################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../global/global/parser.c \
../global/global/utils.c 

OBJS += \
./global/global/parser.o \
./global/global/utils.o 

C_DEPS += \
./global/global/parser.d \
./global/global/utils.d 


# Each subdirectory must supply rules for building sources it contributes
global/global/%.o: ../global/global/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


