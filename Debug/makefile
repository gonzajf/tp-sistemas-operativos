################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/plataforma/subdir.mk
-include src/personaje/subdir.mk
-include src/nivel/Gui/subdir.mk
-include src/nivel/subdir.mk
-include src/memoria/subdir.mk
-include src/leerConfig/subdir.mk
-include src/commons/nivel-gui/subdir.mk
-include src/commons/commons/collections/subdir.mk
-include src/commons/commons/subdir.mk
-include src/commons/Debug/nivel-gui/subdir.mk
-include src/commons/Debug/commons/collections/subdir.mk
-include src/commons/Debug/commons/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: tp-20131c-oiram-repus-corp

dependents:
	-cd /home/utnso/workspace/commons/Debug && $(MAKE) all

# Tool invocations
tp-20131c-oiram-repus-corp: $(OBJS) $(USER_OBJS) /home/utnso/workspace/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/commons/Debug" -o "tp-20131c-oiram-repus-corp" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) tp-20131c-oiram-repus-corp
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/commons/Debug/libcommons.so:

-include ../makefile.targets
