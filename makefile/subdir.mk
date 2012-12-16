################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../adc.c \
../button.c \
../config.c \
../crc8.c \
../display.c \
../gpi.c \
../menu.c \
../symbols8x8.c \
../temp.c 

OBJS += \
./adc.o \
./button.o \
./config.o \
./crc8.o \
./display.o \
./gpi.o \
./menu.o \
./symbols8x8.o \
./temp.o 

C_DEPS += \
./adc.d \
./button.d \
./config.d \
./crc8.d \
./display.d \
./gpi.d \
./menu.d \
./symbols8x8.d \
./temp.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega88p -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


