################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Memoria/memoria.c 

OBJS += \
./Memoria/memoria.o 

C_DEPS += \
./Memoria/memoria.d 


# Each subdirectory must supply rules for building sources it contributes
Memoria/%.o: ../Memoria/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


