/**
 * OpenWare OS - GDT (Global Descriptor Table) Header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef OPENWARE_GDT_H
#define OPENWARE_GDT_H

#include "../include/types.h"

/* GDT entry structure */
typedef struct {
    uint16_t limit_low;     /* Limit bits 0-15 */
    uint16_t base_low;      /* Base bits 0-15 */
    uint8_t  base_middle;   /* Base bits 16-23 */
    uint8_t  access;        /* Access flags */
    uint8_t  granularity;   /* Granularity + limit bits 16-19 */
    uint8_t  base_high;     /* Base bits 24-31 */
} __attribute__((packed)) gdt_entry_t;

/* GDT pointer structure */
typedef struct {
    uint16_t limit;         /* Size of GDT - 1 */
    uint32_t base;          /* Address of first GDT entry */
} __attribute__((packed)) gdt_ptr_t;

/* Segment selectors */
#define GDT_KERNEL_CODE     0x08
#define GDT_KERNEL_DATA     0x10
#define GDT_USER_CODE       0x18
#define GDT_USER_DATA       0x20
#define GDT_TSS             0x28

/* GDT functions */
void gdt_init(void);
void gdt_set_entry(int32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);

#endif /* OPENWARE_GDT_H */
