########################################
# Toolchain and QEMU configuration
########################################

# Compiler toolchain
CC := arm-none-eabi-gcc
LD := arm-none-eabi-gcc
SIZE := arm-none-eabi-size

# Path to QEMU binary
QEMU := /home/peppe/Scrivania/Project/qemu/build/qemu-system-arm

# Target embedded board and CPU
MACHINE := s32k3x8evb
CPU := cortex-m7

# QEMU flags for debugging
QEMU_FLAGS_DBG := -s -S 

########################################
# Project directories
########################################

# FreeRTOS source root
FREERTOS_ROOT := ../FreeRTOS/FreeRTOS

# Demo project directory
DEMO_PROJECT := .

# FreeRTOS kernel directories
KERNEL_DIR := $(FREERTOS_ROOT)/Source
KERNEL_PORT_DIR := $(KERNEL_DIR)/portable/GCC/ARM_CM7/r0p1

# Output directory for build artifacts
OUTPUT_DIR := ./Output

# Optional ARM include
ARM_DIR := ../../qemu/hw/intc/armv7m_nvic.h

########################################
# Output files
########################################

DEMO_NAME := demo
ELF := $(OUTPUT_DIR)/$(DEMO_NAME).elf
MAP := $(OUTPUT_DIR)/$(DEMO_NAME).map

########################################
# Include paths and flags
########################################

INCLUDE_DIRS := -I$(KERNEL_DIR)/include -I$(KERNEL_PORT_DIR)
INCLUDE_DIRS += -I$(DEMO_PROJECT)
INCLUDE_DIRS += -I$(ARM_DIR)

CFLAGS := $(INCLUDE_DIRS)
CFLAGS += -ffreestanding
CFLAGS += -mcpu=$(CPU) -mthumb
CFLAGS += -Wall -Wextra -Wshadow
CFLAGS += -g3 -Os
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -mfpu=fpv5-sp-d16 -mfloat-abi=hard

LDFLAGS := -T ./linker.ld
LDFLAGS += -nostartfiles
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -Xlinker -Map=$(MAP)
LDFLAGS += -Xlinker --gc-sections
LDFLAGS += -mcpu=$(CPU) -mthumb
LDFLAGS += -mfpu=fpv5-sp-d16 -mfloat-abi=hard

VPATH += $(KERNEL_DIR) $(KERNEL_PORT_DIR) $(KERNEL_DIR)/portable/MemMang
VPATH += $(DEMO_PROJECT)

########################################
# Source files
########################################

# FreeRTOS kernel files
SOURCE_FILES := $(KERNEL_DIR)/list.c \
	$(KERNEL_DIR)/tasks.c \
	$(KERNEL_DIR)/timers.c \
	$(KERNEL_DIR)/portable/MemMang/heap_4.c \
	$(KERNEL_PORT_DIR)/port.c \
	$(KERNEL_DIR)/queue.c \
	$(KERNEL_DIR)/event_groups.c \
	$(KERNEL_DIR)/stream_buffer.c

# Demo files
SOURCE_FILES += $(DEMO_PROJECT)/main.c \
	$(DEMO_PROJECT)/lpuart.c \
	$(DEMO_PROJECT)/flexcan.c

# Start-up code
SOURCE_FILES += ./startup.c

# Object files
OBJS := $(SOURCE_FILES:%.c=%.o)
OBJS_NOPATH := $(notdir $(OBJS))
OBJS_OUTPUT := $(OBJS_NOPATH:%.o=$(OUTPUT_DIR)/%.o)

########################################
# Build rules
########################################

all: $(ELF)

$(ELF): $(OBJS_OUTPUT) ./linker.ld Makefile
	@echo "\n--- Linking final ELF ---\n"
	$(LD) $(LDFLAGS) $(OBJS_OUTPUT) -o $(ELF)
	$(SIZE) $(ELF)

$(OUTPUT_DIR)/%.o: %.c Makefile | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

cleanobj:
	rm -f $(OUTPUT_DIR)/*.o

clean:
	rm -rf $(ELF) $(MAP) $(OUTPUT_DIR)/*.o $(OUTPUT_DIR)

########################################
# QEMU targets
########################################

qemu_start:
	$(QEMU) -machine $(MACHINE) -cpu $(CPU) -kernel $(ELF) \
	-monitor none -nographic -serial stdio -d guest_errors

qemu_debug:
	$(QEMU) -machine $(MACHINE) -cpu $(CPU) -kernel $(ELF) \
	-monitor none -nographic -serial stdio $(QEMU_FLAGS_DBG) -d guest_errors -d int,cpu_reset,guest_errors

gdb_start:
	gdb-multiarch $(ELF)
