################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
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
all: libmemoria.so

dependents:
	-cd /home/utnso/workspace/so-commons-library/Debug && $(MAKE) all

# Tool invocations
libmemoria.so: $(OBJS) $(USER_OBJS) /home/utnso/workspace/so-commons-library/Debug/libso-commons-library.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/so-commons-library/Debug" -shared -o "libmemoria.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libmemoria.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/so-commons-library/Debug/libso-commons-library.so:

-include ../makefile.targets
