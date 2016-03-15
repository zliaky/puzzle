################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tagDetect/main.cpp 

OBJS += \
./tagDetect/main.o 

CPP_DEPS += \
./tagDetect/main.d 


# Each subdirectory must supply rules for building sources it contributes
tagDetect/%.o: ../tagDetect/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


