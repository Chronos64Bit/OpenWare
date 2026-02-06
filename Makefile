# ============================================================================
# OpenWare OS - Build System
# Copyright (c) 2026 Ventryx Inc. All rights reserved.
# ============================================================================

# Toolchain (adjust these for your system)
# For Windows with i686-elf cross-compiler:
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld

# If using MinGW on Windows, uncomment these:
# CC = gcc -m32
# LD = ld -m elf_i386

# Directories
BOOT_DIR = boot
KERNEL_DIR = kernel
FS_DIR = fs
INCLUDE_DIR = include
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

# Output files
BOOTLOADER = $(BUILD_DIR)/boot.bin
KERNEL = $(BUILD_DIR)/kernel.bin
ISO = $(BUILD_DIR)/openware.iso

# Compiler flags
CFLAGS = -m32 -ffreestanding -fno-stack-protector -fno-pie -nostdlib \
         -Wall -Wextra -I$(INCLUDE_DIR) -I$(KERNEL_DIR)

# Linker flags
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# Assembler flags
ASFLAGS_16 = -f bin
ASFLAGS_32 = -f elf32

# Source files
KERNEL_C_SRC = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_ASM_SRC = $(KERNEL_DIR)/kernel_entry.asm $(KERNEL_DIR)/isr.asm

# Object files
KERNEL_C_OBJ = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/%.o, $(KERNEL_C_SRC))
KERNEL_ASM_OBJ = $(patsubst $(KERNEL_DIR)/%.asm, $(BUILD_DIR)/%.o, $(KERNEL_ASM_SRC))
KERNEL_OBJ = $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ)

# ============================================================================
# Targets
# ============================================================================

.PHONY: all clean run iso boot kernel dirs

all: dirs $(ISO)

# Create build directories
dirs:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@if not exist $(ISO_DIR) mkdir $(ISO_DIR)
	@if not exist $(ISO_DIR)\boot mkdir $(ISO_DIR)\boot

# Build ISO image
$(ISO): $(BOOTLOADER) $(KERNEL)
	@echo Creating ISO image...
	@copy /b $(BOOTLOADER) + $(KERNEL) $(BUILD_DIR)\os.bin > nul
	@copy $(BUILD_DIR)\os.bin $(ISO_DIR)\boot\os.bin > nul
	@echo Creating bootable ISO with xorriso...
	xorriso -as mkisofs -b boot/os.bin -no-emul-boot -boot-load-size 4 \
	        -o $(ISO) $(ISO_DIR)
	@echo ISO created: $(ISO)

# Build bootloader (stage1 + stage2)
boot: dirs $(BOOTLOADER)

$(BOOTLOADER): $(BOOT_DIR)/stage1.asm $(BOOT_DIR)/stage2.asm
	@echo Assembling Stage 1 bootloader...
	$(AS) $(ASFLAGS_16) $(BOOT_DIR)/stage1.asm -o $(BUILD_DIR)/stage1.bin
	@echo Assembling Stage 2 bootloader...
	$(AS) $(ASFLAGS_16) $(BOOT_DIR)/stage2.asm -o $(BUILD_DIR)/stage2.bin
	@echo Combining bootloader stages...
	@copy /b $(BUILD_DIR)\stage1.bin + $(BUILD_DIR)\stage2.bin $(BOOTLOADER) > nul

# Build kernel
kernel: dirs $(KERNEL)

$(KERNEL): $(KERNEL_OBJ)
	@echo Linking kernel...
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/kernel.elf $(KERNEL_OBJ)
	objcopy -O binary $(BUILD_DIR)/kernel.elf $(KERNEL)
	@echo Padding kernel to sector boundary...
	@python -c "import os; f=open('$(KERNEL)','ab'); f.write(b'\0'*(512-os.path.getsize('$(KERNEL)')%%512)); f.close()" 2>nul || echo Padding skipped

# Compile C files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble kernel ASM files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.asm
	@echo Assembling $<...
	$(AS) $(ASFLAGS_32) $< -o $@

# Run in QEMU
run: $(ISO)
	@echo Launching OpenWare in QEMU...
	qemu-system-i386 -cdrom $(ISO)

# Run with debug
debug: $(ISO)
	@echo Launching OpenWare in QEMU with debug...
	qemu-system-i386 -cdrom $(ISO) -s -S

# Clean build artifacts
clean:
	@echo Cleaning build directory...
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@echo Clean complete.

# ============================================================================
# Help
# ============================================================================
help:
	@echo OpenWare OS Build System
	@echo ========================
	@echo.
	@echo Targets:
	@echo   all     - Build everything (default)
	@echo   boot    - Build bootloader only
	@echo   kernel  - Build kernel only
	@echo   iso     - Create ISO image
	@echo   run     - Build and run in QEMU
	@echo   debug   - Build and run in QEMU with GDB server
	@echo   clean   - Remove build artifacts
	@echo   help    - Show this help message
	@echo.
	@echo Requirements:
	@echo   - NASM (Netwide Assembler)
	@echo   - i686-elf-gcc (Cross compiler) or MinGW
	@echo   - xorriso (for ISO creation)
	@echo   - QEMU (optional, for testing)
