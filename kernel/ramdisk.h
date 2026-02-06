/**
 * OpenWare OS - Embedded Ramdisk Driver header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef RAMDISK_H
#define RAMDISK_H

#include "../include/types.h"

/* Exposed by linker from objcopy */
extern char _binary_ramdisk_img_start[];
extern char _binary_ramdisk_img_end[];
extern char _binary_ramdisk_img_size[];

void ramdisk_read(uint32_t lba, uint8_t sectors, uint8_t* buffer);
void ramdisk_write(uint32_t lba, uint8_t sectors, uint8_t* buffer);

#endif
