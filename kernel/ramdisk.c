/**
 * OpenWare OS - Embedded Ramdisk Driver
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "ramdisk.h"
#include "../include/memory.h"
#include "vga.h"

/* Helper to get ramdisk size */
static uint32_t ramdisk_get_size(void) {
    return (uint32_t)_binary_ramdisk_img_end - (uint32_t)_binary_ramdisk_img_start;
}

void ramdisk_read(uint32_t lba, uint8_t sectors, uint8_t* buffer) {
    uint32_t offset = lba * 512;
    uint32_t size = sectors * 512;
    uint32_t max_size = ramdisk_get_size();
    
    if (offset + size > max_size) {
        /* Read beyond end - clamp or error */
        if (offset >= max_size) return;
        size = max_size - offset;
    }
    
    kmemcpy(buffer, (uint8_t*)_binary_ramdisk_img_start + offset, size);
}

void ramdisk_write(uint32_t lba, uint8_t sectors, uint8_t* buffer) {
    uint32_t offset = lba * 512;
    uint32_t size = sectors * 512;
    uint32_t max_size = ramdisk_get_size();
    
    if (offset + size > max_size) {
        if (offset >= max_size) return;
        size = max_size - offset;
    }
    
    /* Write to memory */
    kmemcpy((uint8_t*)_binary_ramdisk_img_start + offset, buffer, size);
}
