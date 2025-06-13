# ClaudeOS Makefile - Day 2 Restart (Simple Version)

CC = gcc
AS = nasm
LD = ld

# Compiler flags for 32-bit kernel
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c

# Assembler flags
ASFLAGS = -f elf32

# Linker flags
LDFLAGS = -m elf_i386 -T linker.ld

# Object files
OBJS = build/entry.o build/kernel.o

# Build directory
BUILD_DIR = build

.PHONY: all clean run run-kernel

all: $(BUILD_DIR)/kernel.bin

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile kernel entry point
$(BUILD_DIR)/entry.o: kernel/entry.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile kernel C code
$(BUILD_DIR)/kernel.o: kernel/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Link kernel
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# Run kernel in QEMU
run-kernel: $(BUILD_DIR)/kernel.bin
	qemu-system-i386 -kernel $< -m 32M

# Run basic bootloader
run:
	nasm -f bin boot/boot.asm -o $(BUILD_DIR)/bootloader.bin
	qemu-system-i386 -drive file=$(BUILD_DIR)/bootloader.bin,format=raw -m 16M

# Run protected mode bootloader
run-protected:
	nasm -f bin boot/boot_protected.asm -o $(BUILD_DIR)/bootloader_protected.bin
	qemu-system-i386 -drive file=$(BUILD_DIR)/bootloader_protected.bin,format=raw -m 16M

# Clean build files
clean:
	rm -rf $(BUILD_DIR)/*

# Debug information
info:
	@echo "Compiler: $(CC)"
	@echo "Assembler: $(AS)"
	@echo "Linker: $(LD)"
	@echo "Objects: $(OBJS)"