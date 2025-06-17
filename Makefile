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
OBJS = build/entry.o build/kernel.o build/gdt.o build/gdt_flush.o build/idt.o build/idt_flush.o build/isr.o build/isr_asm.o build/pic.o build/io.o build/timer.o build/keyboard.o build/serial.o build/pmm.o build/vmm.o build/paging.o build/heap.o build/process.o build/context_switch.o build/string.o build/syscall.o build/syscall_handler.o build/simplefs.o

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

# Compile GDT C code
$(BUILD_DIR)/gdt.o: kernel/gdt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile GDT flush assembly
$(BUILD_DIR)/gdt_flush.o: kernel/gdt_flush.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile IDT C code
$(BUILD_DIR)/idt.o: kernel/idt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile IDT flush assembly
$(BUILD_DIR)/idt_flush.o: kernel/idt_flush.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile ISR C code
$(BUILD_DIR)/isr.o: kernel/isr.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile ISR assembly
$(BUILD_DIR)/isr_asm.o: kernel/isr.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile PIC C code
$(BUILD_DIR)/pic.o: kernel/pic.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile I/O assembly
$(BUILD_DIR)/io.o: kernel/io.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile Timer C code
$(BUILD_DIR)/timer.o: kernel/timer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile Keyboard C code
$(BUILD_DIR)/keyboard.o: kernel/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile Serial C code
$(BUILD_DIR)/serial.o: kernel/serial.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile PMM C code
$(BUILD_DIR)/pmm.o: kernel/pmm.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile VMM C code
$(BUILD_DIR)/vmm.o: kernel/vmm.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile Paging assembly
$(BUILD_DIR)/paging.o: kernel/paging.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile Heap C code
$(BUILD_DIR)/heap.o: kernel/heap.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile Process C code
$(BUILD_DIR)/process.o: kernel/process.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile Context Switch assembly
$(BUILD_DIR)/context_switch.o: kernel/context_switch.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile String C code
$(BUILD_DIR)/string.o: kernel/string.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile System Call C code
$(BUILD_DIR)/syscall.o: kernel/syscall.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Compile System Call Handler assembly
$(BUILD_DIR)/syscall_handler.o: kernel/syscall_handler.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile SimpleFS C code
$(BUILD_DIR)/simplefs.o: fs/simplefs.c | $(BUILD_DIR)
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