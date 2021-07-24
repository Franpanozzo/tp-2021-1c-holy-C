################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mi_ram_hq.c \
../paginacion.c \
../segmentacion.c 

OBJS += \
./mi_ram_hq.o \
./paginacion.o \
./segmentacion.o 

C_DEPS += \
./mi_ram_hq.d \
./paginacion.d \
./segmentacion.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-1c-holy-C/bibliotecas/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


