/**
 * OpenWare OS - ATA (Hard Disk) Driver Header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef ATA_H
#define ATA_H

#include "../include/types.h"

/* ATA Standard Ports (Primary Channel) */
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERROR        0x1F1
#define ATA_PRIMARY_SEC_COUNT    0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_STATUS       0x1F7 /* Read */
#define ATA_PRIMARY_COMMAND      0x1F7 /* Write */

/* ATA Standard Ports (Secondary Channel) */
#define ATA_SECONDARY_DATA       0x170
#define ATA_SECONDARY_ERROR      0x171
#define ATA_SECONDARY_SEC_COUNT  0x172
#define ATA_SECONDARY_LBA_LO     0x173
#define ATA_SECONDARY_LBA_MID    0x174
#define ATA_SECONDARY_LBA_HI     0x175
#define ATA_SECONDARY_DRIVE_HEAD 0x176
#define ATA_SECONDARY_STATUS     0x177
#define ATA_SECONDARY_COMMAND    0x177

/* ATA Status Register Bits */
#define ATA_SR_BSY     0x80    /* Busy */
#define ATA_SR_DRDY    0x40    /* Drive Ready */
#define ATA_SR_DF      0x20    /* Drive Write Fault */
#define ATA_SR_DSC     0x10    /* Drive Seek Complete */
#define ATA_SR_DRQ     0x08    /* Data Request Ready */
#define ATA_SR_CORR    0x04    /* Corrected Data */
#define ATA_SR_IDX     0x02    /* Index */
#define ATA_SR_ERR     0x01    /* Error */

/* ATA Commands */
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY        0xEC

/* Drives */
#define ATA_MASTER     0xA0
#define ATA_SLAVE      0xB0

void ata_init(void);
void ata_read_sectors(uint32_t lba, uint8_t sectors, uint8_t* buffer);
void ata_write_sectors(uint32_t lba, uint8_t sectors, uint8_t* buffer);
void ata_soft_reset(void);

#endif
