/**
 * OpenWare OS - Kernel Linker Script
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

/* Kernel entry point */
ENTRY(_start)

/* Kernel is loaded at 1MB */
KERNEL_LOAD_ADDR = 0x100000;

SECTIONS
{
    /* Start at 1MB */
    . = KERNEL_LOAD_ADDR;

    /* Code section */
    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /* Kernel end marker */
    __kernel_end = .;

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame*)
    }
}
