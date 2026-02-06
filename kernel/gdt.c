/**
 * OpenWare OS - GDT (Global Descriptor Table) Implementation
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "gdt.h"

/* GDT entries - 6 entries for kernel */
static gdt_entry_t gdt_entries[6];
static gdt_ptr_t gdt_ptr;

/* External assembly function to load the GDT */
extern void gdt_flush(uint32_t);

/**
 * Set a GDT entry
 */
void gdt_set_entry(int32_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt_entries[index].base_low = (base & 0xFFFF);
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high = (base >> 24) & 0xFF;

    gdt_entries[index].limit_low = (limit & 0xFFFF);
    gdt_entries[index].granularity = (limit >> 16) & 0x0F;
    gdt_entries[index].granularity |= granularity & 0xF0;
    gdt_entries[index].access = access;
}

/**
 * Initialize the GDT
 */
void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    /* Null segment */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code segment: base=0, limit=4GB, code, ring 0 */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Kernel data segment: base=0, limit=4GB, data, ring 0 */
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* User code segment: base=0, limit=4GB, code, ring 3 */
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* User data segment: base=0, limit=4GB, data, ring 3 */
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* TSS (Task State Segment) - will be set up later */
    gdt_set_entry(5, 0, 0, 0, 0);

    /* Load the GDT */
    gdt_flush((uint32_t)&gdt_ptr);
}
