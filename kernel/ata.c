/**
 * OpenWare OS - ATA (Hard Disk) Driver Implementation
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "ata.h"
#include "vga.h"

/* Helper for port I/O */
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

/* Wait for 400ns */
static void ata_wait_io(void) {
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
}

/* Poll status */
static void ata_wait_busy(void) {
    while (inb(ATA_PRIMARY_STATUS) & ATA_SR_BSY);
}

static void ata_wait_ready(void) {
    while (!(inb(ATA_PRIMARY_STATUS) & ATA_SR_DRDY));
}

/**
 * Initialize ATA driver
 */
void ata_init(void) {
    // Identify drive would go here
}

/**
 * Read sectors using PIO mode (28-bit LBA)
 */
void ata_read_sectors(uint32_t lba, uint8_t sectors, uint8_t* buffer) {
    ata_wait_busy();

    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_ERROR, 0x00);
    outb(ATA_PRIMARY_SEC_COUNT, sectors);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)(lba));
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);

    uint16_t* target = (uint16_t*)buffer;

    for (int i = 0; i < sectors; i++) {
        ata_wait_busy();
        ata_wait_ready();

        for (int j = 0; j < 256; j++) {
            target[j + (i * 256)] = inw(ATA_PRIMARY_DATA);
        }
    }
}

/**
 * Write sectors using PIO mode
 */
void ata_write_sectors(uint32_t lba, uint8_t sectors, uint8_t* buffer) {
    ata_wait_busy();

    outb(ATA_PRIMARY_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_ERROR, 0x00);
    outb(ATA_PRIMARY_SEC_COUNT, sectors);
    outb(ATA_PRIMARY_LBA_LO, (uint8_t)(lba));
    outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HI, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);

    uint16_t* target = (uint16_t*)buffer;

    for (int i = 0; i < sectors; i++) {
        ata_wait_busy();
        ata_wait_ready();

        for (int j = 0; j < 256; j++) {
            outw(ATA_PRIMARY_DATA, target[j + (i * 256)]);
        }
    }
    
    // Cache flush command
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_CACHE_FLUSH);
    ata_wait_busy();
}
