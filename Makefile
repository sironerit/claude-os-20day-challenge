# Claude OS Makefile
# 70日チャレンジ - Day 1作成

# コンパイラとツール設定
CC = gcc
AS = nasm
LD = ld

# フラグ設定
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

# ディレクトリ設定
BOOT_DIR = boot
KERNEL_DIR = kernel
BUILD_DIR = build
ISO_DIR = iso

# ターゲット設定
BOOTLOADER = $(BUILD_DIR)/bootloader.bin
BOOTLOADER_PROTECTED = $(BUILD_DIR)/bootloader_protected.bin
KERNEL = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/claude-os.img
ISO_IMAGE = $(BUILD_DIR)/claude-os.iso

# デフォルトターゲット
all: $(OS_IMAGE)

# ビルドディレクトリ作成
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ブートローダービルド（Day 2実装）
$(BOOTLOADER): $(BUILD_DIR) $(BOOT_DIR)/boot.asm
	@echo "Building basic bootloader..."
	$(AS) -f bin $(BOOT_DIR)/boot.asm -o $(BOOTLOADER)

# プロテクトモード対応ブートローダー
$(BOOTLOADER_PROTECTED): $(BUILD_DIR) $(BOOT_DIR)/boot_protected.asm
	@echo "Building protected mode bootloader..."
	$(AS) -f bin $(BOOT_DIR)/boot_protected.asm -o $(BOOTLOADER_PROTECTED)

# カーネルビルド（Day 4実装: GDT + IDT + 割り込み）
$(KERNEL): $(BUILD_DIR) $(KERNEL_DIR)/kernel.c $(KERNEL_DIR)/entry.asm $(KERNEL_DIR)/gdt.c $(KERNEL_DIR)/gdt_flush.asm $(KERNEL_DIR)/idt.c $(KERNEL_DIR)/idt_flush.asm $(KERNEL_DIR)/isr.asm $(KERNEL_DIR)/isr.c $(KERNEL_DIR)/io.asm
	@echo "Building kernel with GDT, IDT, and interrupts..."
	$(AS) $(ASFLAGS) $(KERNEL_DIR)/entry.asm -o $(BUILD_DIR)/entry.o
	$(AS) $(ASFLAGS) $(KERNEL_DIR)/gdt_flush.asm -o $(BUILD_DIR)/gdt_flush.o
	$(AS) $(ASFLAGS) $(KERNEL_DIR)/idt_flush.asm -o $(BUILD_DIR)/idt_flush.o
	$(AS) $(ASFLAGS) $(KERNEL_DIR)/isr.asm -o $(BUILD_DIR)/isr.o
	$(AS) $(ASFLAGS) $(KERNEL_DIR)/io.asm -o $(BUILD_DIR)/io.o
	$(CC) $(CFLAGS) $(KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o
	$(CC) $(CFLAGS) $(KERNEL_DIR)/gdt.c -o $(BUILD_DIR)/gdt.o
	$(CC) $(CFLAGS) $(KERNEL_DIR)/idt.c -o $(BUILD_DIR)/idt.o
	$(CC) $(CFLAGS) $(KERNEL_DIR)/isr.c -o $(BUILD_DIR)/isr_c.o
	$(LD) $(LDFLAGS) -o $(KERNEL) $(BUILD_DIR)/entry.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/gdt.o $(BUILD_DIR)/gdt_flush.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/idt_flush.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/isr_c.o $(BUILD_DIR)/io.o

# OSイメージ作成（Day 3: ブートローダー+カーネル統合）
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL)
	@echo "Creating OS image with bootloader and kernel..."
	dd if=$(BOOTLOADER) of=$(OS_IMAGE) bs=512 count=1
	dd if=$(KERNEL) of=$(OS_IMAGE) bs=512 seek=1

# QEMU実行（基本ブートローダー）
run: $(BOOTLOADER)
	qemu-system-i386 -drive file=$(BOOTLOADER),format=raw -m 16M

# QEMU実行（プロテクトモード）
run-protected: $(BOOTLOADER_PROTECTED)
	qemu-system-i386 -drive file=$(BOOTLOADER_PROTECTED),format=raw -m 16M

# QEMU実行（カーネル単体テスト）
run-kernel: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 16M

# QEMU実行（完全なOSイメージ）
run-os: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw -m 16M

# ISO作成
iso: $(ISO_IMAGE)

$(ISO_IMAGE): $(OS_IMAGE)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(OS_IMAGE) $(ISO_DIR)/boot/
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $(ISO_IMAGE) $(ISO_DIR)

# クリーンアップ
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(ISO_DIR)

# デバッグ情報
info:
	@echo "Claude OS 70日チャレンジ"
	@echo "Day 2: ブートローダー実装"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Compiler: $(CC)"
	@echo "Assembler: $(AS)"

# ヘルプ
help:
	@echo "Claude OS Makefile Commands:"
	@echo "  make all    - フルビルド"
	@echo "  make run    - QEMU実行"
	@echo "  make iso    - ISO作成"
	@echo "  make clean  - クリーンアップ"
	@echo "  make info   - ビルド情報表示"
	@echo "  make help   - このヘルプ表示"

.PHONY: all run iso clean info help