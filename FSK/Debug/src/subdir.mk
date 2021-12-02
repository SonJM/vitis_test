################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/helloworld.c \
../src/peri_iic_ctrl.c \
../src/platform.c 

OBJS += \
./src/helloworld.o \
./src/peri_iic_ctrl.o \
./src/platform.o 

C_DEPS += \
./src/helloworld.d \
./src/peri_iic_ctrl.d \
./src/platform.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -IE:/PROJ_Archive/release/FMC_LCD_FSK_1_2_B/Firmware/vitis_2020_1/FMC_FSK/export/FMC_FSK/sw/FMC_FSK/standalone_domain/bspinclude/include -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


