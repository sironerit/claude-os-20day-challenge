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
KERNEL = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/claude-os.img
ISO_IMAGE = $(BUILD_DIR)/claude-os.iso

# デフォルトターゲット
all: $(OS_IMAGE)

# ビルドディレクトリ作成
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ブートローダービルド（Day 2-7で実装予定）
$(BOOTLOADER): $(BUILD_DIR)
	@echo "Day 2-7でブートローダーを実装します"
	# $(AS) $(ASFLAGS) $(BOOT_DIR)/boot.asm -o $(BUILD_DIR)/boot.o
	# $(LD) $(LDFLAGS) -o $(BOOTLOADER) $(BUILD_DIR)/boot.o

# カーネルビルド（Day 8-14で実装予定）
$(KERNEL): $(BUILD_DIR)
	@echo "Day 8-14でカーネルを実装します"
	# $(CC) $(CFLAGS) $(KERNEL_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o
	# $(LD) $(LDFLAGS) -o $(KERNEL) $(BUILD_DIR)/kernel.o

# OSイメージ作成
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL)
	@echo "Day 1: プロジェクト構造作成完了"
	@echo "次回はブートローダー実装から開始します"
	touch $(OS_IMAGE)

# QEMU実行
run: $(OS_IMAGE)
	qemu-system-x86_64 -drive file=$(OS_IMAGE),format=raw

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
	@echo "Day 1: 環境構築・プロジェクト初期化"
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