#!/bin/bash
# ============================================================================
# OpenWare OS - WSL Build Script
# Copyright (c) 2026 Ventryx Inc. All rights reserved.
# ============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Banner
echo -e "${CYAN}"
echo "  ╔═══════════════════════════════════════════╗"
echo "  ║        OpenWare OS Build System           ║"
echo "  ║           WSL Edition v1.0                ║"
echo "  ╚═══════════════════════════════════════════╝"
echo -e "${NC}"

# Get the script's directory (works when called from Windows path)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${BLUE}[INFO]${NC} Working directory: $SCRIPT_DIR"

# Directories
BUILD_DIR="build"
ISO_DIR="$BUILD_DIR/iso"
BOOT_DIR="boot"
KERNEL_DIR="kernel"

# Output files
ISO_FILE="$BUILD_DIR/openware.iso"

# ============================================================================
# Check Dependencies
# ============================================================================
check_dependencies() {
    echo -e "${BLUE}[INFO]${NC} Checking dependencies..."
    
    local missing=()
    
    # Check for NASM
    if ! command -v nasm &> /dev/null; then
        missing+=("nasm")
    fi
    
    # Check for i686-elf-gcc (try multiple names)
    if command -v i686-elf-gcc &> /dev/null; then
        CC="i686-elf-gcc"
        LD="i686-elf-ld"
        OBJCOPY="i686-elf-objcopy"
    elif command -v i686-linux-gnu-gcc &> /dev/null; then
        CC="i686-linux-gnu-gcc"
        LD="i686-linux-gnu-ld"
        OBJCOPY="i686-linux-gnu-objcopy"
    elif command -v gcc &> /dev/null; then
        # Fallback to native gcc with -m32
        CC="gcc"
        LD="ld"
        OBJCOPY="objcopy"
        echo -e "${YELLOW}[WARN]${NC} Using native gcc - may need multilib support"
    else
        missing+=("i686-elf-gcc or gcc")
    fi
    
    # Check for xorriso or genisoimage
    if command -v xorriso &> /dev/null; then
        ISO_TOOL="xorriso"
    elif command -v genisoimage &> /dev/null; then
        ISO_TOOL="genisoimage"
    elif command -v mkisofs &> /dev/null; then
        ISO_TOOL="mkisofs"
    else
        missing+=("xorriso (or genisoimage)")
    fi
    
    if [ ${#missing[@]} -ne 0 ]; then
        echo -e "${RED}[ERROR]${NC} Missing dependencies: ${missing[*]}"
        echo ""
        echo -e "${YELLOW}Install them with:${NC}"
        echo "  sudo apt update"
        echo "  sudo apt install nasm xorriso"
        echo ""
        echo "For cross-compiler (recommended):"
        echo "  sudo apt install gcc-i686-linux-gnu"
        echo ""
        echo "Or for native 32-bit compilation:"
        echo "  sudo apt install gcc-multilib"
        exit 1
    fi
    
    echo -e "${GREEN}[OK]${NC} All dependencies found"
    echo -e "  CC: $CC"
    echo -e "  LD: $LD"
    echo -e "  ISO Tool: $ISO_TOOL"
}

# ============================================================================
# Create directories
# ============================================================================
create_dirs() {
    echo -e "${BLUE}[INFO]${NC} Creating build directories..."
    mkdir -p "$BUILD_DIR"
    mkdir -p "$ISO_DIR/boot/grub"
}

# ============================================================================
# Build Bootloader
# ============================================================================
build_bootloader() {
    echo -e "${BLUE}[INFO]${NC} Building bootloader..."
    
    # Stage 1 (boot sector)
    echo -e "  ${CYAN}→${NC} Assembling stage1.asm..."
    nasm -f bin "$BOOT_DIR/stage1.asm" -o "$BUILD_DIR/stage1.bin"
    
    # Stage 2 (protected mode loader)
    echo -e "  ${CYAN}→${NC} Assembling stage2.asm..."
    nasm -f bin "$BOOT_DIR/stage2.asm" -o "$BUILD_DIR/stage2.bin"
    
    # Combine stages
    echo -e "  ${CYAN}→${NC} Combining bootloader stages..."
    cat "$BUILD_DIR/stage1.bin" "$BUILD_DIR/stage2.bin" > "$BUILD_DIR/boot.bin"
    
    echo -e "${GREEN}[OK]${NC} Bootloader built: $BUILD_DIR/boot.bin"
}

# ============================================================================
# Build Kernel
# ============================================================================
build_kernel() {
    echo -e "${BLUE}[INFO]${NC} Building kernel..."
    
    # Compiler flags
    CFLAGS="-m32 -ffreestanding -fno-stack-protector -fno-pie -nostdlib"
    CFLAGS="$CFLAGS -Wall -Wextra -I./include -I./$KERNEL_DIR"
    
    if [ "$DEV_BUILD" = true ]; then
        echo -e "${YELLOW}[INFO]${NC} DEV BUILD ENABLED (Debug Symbols + Logging)"
        CFLAGS="$CFLAGS -g -DDEBUG"
    fi
    
    # Assemble kernel entry
    echo -e "  ${CYAN}→${NC} Assembling kernel_entry.asm..."
    nasm -f elf32 "$KERNEL_DIR/kernel_entry.asm" -o "$BUILD_DIR/kernel_entry.o"
    
    # Assemble ISR handlers
    echo -e "  ${CYAN}→${NC} Assembling isr.asm..."
    nasm -f elf32 "$KERNEL_DIR/isr.asm" -o "$BUILD_DIR/isr.o"
    
    # Compile all C files from kernel and fs
    SRCS="$(find "$KERNEL_DIR" fs -name "*.c" 2>/dev/null)"
    
    for src in $SRCS; do
        if [ -f "$src" ]; then
            name=$(basename "$src" .c)
            # Handle duplicate names if any (simple approach)
            if [ -f "$BUILD_DIR/$name.o" ]; then
                name="${name}_$(basename $(dirname "$src"))"
            fi
            
            echo -e "  ${CYAN}→${NC} Compiling $name.c..."
            $CC $CFLAGS -c "$src" -o "$BUILD_DIR/$name.o"
        fi
    done
    
    # Link kernel
    echo -e "  ${CYAN}→${NC} Linking kernel..."
    
    # Gather all object files (entry first!)
    OBJS="$BUILD_DIR/kernel_entry.o $BUILD_DIR/isr.o $BUILD_DIR/ramdisk_data.o"
    for obj in "$BUILD_DIR"/*.o; do
        case "$obj" in
            *kernel_entry.o|*isr.o|*ramdisk_data.o) ;;  # Already added
            *) OBJS="$OBJS $obj" ;;
        esac
    done
    
    $LD -m elf_i386 -T linker.ld -nostdlib -o "$BUILD_DIR/kernel.elf" $OBJS
    
    # Convert to flat binary
    echo -e "  ${CYAN}→${NC} Creating kernel binary..."
    $OBJCOPY -O binary "$BUILD_DIR/kernel.elf" "$BUILD_DIR/kernel.bin"
    
    # Pad to sector boundary
    SIZE=$(stat -c%s "$BUILD_DIR/kernel.bin")
    PADDED_SIZE=$(( (SIZE + 511) / 512 * 512 ))
    if [ $SIZE -ne $PADDED_SIZE ]; then
        dd if=/dev/zero bs=1 count=$((PADDED_SIZE - SIZE)) >> "$BUILD_DIR/kernel.bin" 2>/dev/null
    fi
    
    echo -e "${GREEN}[OK]${NC} Kernel built: $BUILD_DIR/kernel.bin ($(stat -c%s "$BUILD_DIR/kernel.bin") bytes)"
}

# ============================================================================
# Create ISO Image
# ============================================================================
create_iso() {
    echo -e "${BLUE}[INFO]${NC} Creating ISO image..."
    
    # Combine bootloader and kernel into single binary
    cat "$BUILD_DIR/boot.bin" "$BUILD_DIR/kernel.bin" > "$BUILD_DIR/os.bin"
    
    # Pad to 1.44MB (floppy size) for emulation
    # We use truncate to make it fast
    truncate -s 1474560 "$BUILD_DIR/os.bin"
    
    cp "$BUILD_DIR/os.bin" "$ISO_DIR/boot/os.bin"
    
    # Create bootable ISO using Floppy Emulation
    # The BIOS will present this image as Drive A: (Floppy)
    echo -e "  ${CYAN}→${NC} Creating ISO with floppy emulation..."
    
    xorriso -as mkisofs \
        -b boot/os.bin \
        -o "$ISO_FILE" \
        "$ISO_DIR" 2>&1 | grep -v "^xorriso\|^Drive\|^Media"
    
    echo -e "${GREEN}[OK]${NC} ISO created: $ISO_FILE ($(stat -c%s "$ISO_FILE") bytes)"
}

# ============================================================================
# Create Ramdisk Image (Embedded)
# ============================================================================
create_ramdisk() {
    echo -e "${BLUE}[INFO]${NC} Creating embedded ramdisk..."
    
    # 256KB Ramdisk
    dd if=/dev/zero of="$BUILD_DIR/ramdisk.img" bs=1k count=256 status=none
    
    # Format as FAT12 (Small) or FAT32?
    # FAT32 requires minimum clusters. 256KB is too small for FAT32 usually.
    # We will use FAT12/16 (mkfs.vfat handles it automatically based on size usually)
    # Force minimal FAT32 if possible, or support FAT12 in driver?
    # My driver expects BPB structure which is mostly compatible, but FAT offset calculation differs.
    # Let's try standard mkfs.vfat, it likely picks FAT12. 
    # My FAT32 driver might fail if it's FAT12.
    # TRICK: Make it 33MB sparse file? No, embedded.
    # Workaround: Use FAT32 with forced params or update driver to support FAT12 BPB?
    # Actually, easiest is to Create a "Larger" image but cut it? No.
    # Let's try to format as FAT32 explicitly.
    # mkfs.vfat -F 32 -I ... might complain.
    
    if command -v mkfs.vfat >/dev/null 2>&1; then
        mkfs.vfat -F 32 -I "$BUILD_DIR/ramdisk.img" || mkfs.vfat -I "$BUILD_DIR/ramdisk.img"
        
        # Add test file
        if command -v mcopy >/dev/null 2>&1; then
            echo "Welcome to OpenWare!" > "$BUILD_DIR/README.TXT"
            mcopy -i "$BUILD_DIR/ramdisk.img" "$BUILD_DIR/README.TXT" ::/README.TXT
        fi
        
        # Convert to Object File
        # We cd into build dir to keep symbols clean (_binary_ramdisk_img_start)
        echo -e "  ${CYAN}→${NC} Converting to ELF object..."
        (cd "$BUILD_DIR" && objcopy -I binary -O elf32-i386 -B i386 "ramdisk.img" "ramdisk_data.o")
    else
        echo -e "${RED}[WARN]${NC} mkfs.vfat not found. Ramdisk will be empty zeros."
        # Create dummy object
        touch "$BUILD_DIR/ramdisk.img"
        (cd "$BUILD_DIR" && objcopy -I binary -O elf32-i386 -B i386 "ramdisk.img" "ramdisk_data.o")
    fi
}

# ============================================================================
# Run in QEMU
# ============================================================================
run_qemu() {
    echo -e "${BLUE}[INFO]${NC} Launching in QEMU..."
    
    # Check for QEMU
    QEMU=""
    if command -v qemu-system-i386 &> /dev/null; then
        QEMU="qemu-system-i386"
    elif command -v qemu-system-x86_64 &> /dev/null; then
        QEMU="qemu-system-x86_64"
    else
        # ... logic ...
        return
    fi
    
    $QEMU -cdrom "$ISO_FILE" -m 128
}

# ============================================================================
# Clean
# ============================================================================
clean() {
    echo -e "${BLUE}[INFO]${NC} Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}[OK]${NC} Clean complete"
}

# ============================================================================
# Main
# ============================================================================
case "${1:-all}" in
    all)
        check_dependencies
        create_dirs
        create_ramdisk
        build_bootloader
        build_kernel
        create_iso
        echo ""
        echo -e "${GREEN}╔═══════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║         BUILD SUCCESSFUL!                 ║${NC}"
        echo -e "${GREEN}╚═══════════════════════════════════════════╝${NC}"
        echo ""
        echo -e "  ISO: ${CYAN}$ISO_FILE${NC}"
        echo ""
        echo "  Run with QEMU:"
        echo "    ./build_wsl.sh run"
        echo ""
        echo "  Or on Windows:"
        echo "    qemu-system-i386 -cdrom build/openware.iso"
        ;;
    dev)
        DEV_BUILD=true
        ISO_FILE="$BUILD_DIR/openware-dev.iso"
        check_dependencies
        create_dirs
        create_ramdisk
        build_bootloader
        build_kernel
        create_iso
        echo ""
        echo -e "${GREEN}╔═══════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║      DEV BUILD SUCCESSFUL!                ║${NC}"
        echo -e "${GREEN}╚═══════════════════════════════════════════╝${NC}"
        echo -e "  ISO: ${CYAN}$ISO_FILE${NC}"
        ;;
    boot)
        check_dependencies
        create_dirs
        build_bootloader
        ;;
    kernel)
        check_dependencies
        create_dirs
        build_kernel
        ;;
    iso)
        check_dependencies
        create_iso
        ;;
    run)
        run_qemu
        ;;
    clean)
        clean
        ;;
    install-deps)
        echo -e "${BLUE}[INFO]${NC} Installing build dependencies..."
        sudo apt update
        sudo apt install -y nasm xorriso grub-pc-bin grub-common gcc-i686-linux-gnu qemu-system-x86 dosfstools mtools
        echo -e "${GREEN}[OK]${NC} Dependencies installed!"
        ;;
    *)
        echo "Usage: $0 {all|boot|kernel|iso|run|clean|install-deps}"
        echo ""
        echo "  all          - Build everything (default)"
        echo "  boot         - Build bootloader only"
        echo "  kernel       - Build kernel only"
        echo "  iso          - Create ISO image"
        echo "  run          - Run in QEMU"
        echo "  clean        - Remove build artifacts"
        echo "  install-deps - Install required apt packages"
        exit 1
        ;;
esac
