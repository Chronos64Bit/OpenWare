/**
 * OpenWare OS - Kernel Main
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 * 
 * Main kernel entry point and initialization
 */

#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "shell.h"
#include "memory.h"
#include "ata.h"
#include "version.h"
#include "vbe.h"
#include "mouse.h"
#include "ui.h"


/**
 * Print system initialization status (Graphics Version)
 */
static void print_status_graphics(const char* component, bool success) {
    vbe_print("  [", COLOR_WHITE);
    if (success) {
        vbe_print(" OK ", COLOR_GREEN);
    } else {
        vbe_print("FAIL", COLOR_RED);
    }
    vbe_print("] ", COLOR_WHITE);
    vbe_print(component, COLOR_WHITE);
    vbe_print("\n", COLOR_WHITE);
}

/**
 * Kernel main entry point
 */
void kmain(void) {
    /* Initialize VBE Graphics */
    vbe_init();
    vbe_enable_double_buffering();
    
    // Banner
    vbe_print("\n  ____                __          __            \n", COLOR_CYAN);
    vbe_print(" / __ \\___  ___ ___  / /  ___ ___/ /__          \n", COLOR_CYAN);
    vbe_print("/ /_/ / _ \\/ -_) _ \\/ _ \\/ -_) _  / -_)         \n", COLOR_CYAN);
    vbe_print("\\____/ .__/\\__/_//_/_.__/\\__/\\_,_/\\__/          \n", COLOR_CYAN);
    vbe_print("    /_/                                          \n", COLOR_CYAN);
    
    vbe_print("\n         OpenWare OS v" OS_VERSION_STRING " \"Genesis\"\n", COLOR_GREEN);
    vbe_print("         Copyright (c) 2026 Ventryx Inc.\n", COLOR_GRAY);
    vbe_print("         All rights reserved.\n\n", COLOR_GRAY);
    
    vbe_print("Initializing system components in Graphics Mode...\n\n", COLOR_WHITE);
    
    /* Initialize GDT */
    gdt_init();
    print_status_graphics("Global Descriptor Table (GDT)", true);
    
    /* Initialize IDT */
    idt_init();
    print_status_graphics("Interrupt Descriptor Table (IDT)", true);
    
    /* Initialize keyboard */
    keyboard_init();
    print_status_graphics("PS/2 Keyboard Driver", true);

    /* Initialize mouse */
    mouse_init();
    print_status_graphics("PS/2 Mouse Driver", true);

    /* Initialize memory */
    memory_init();
    print_status_graphics("Memory Manager (Heap)", true);

    /* Initialize ATA */
    ata_init();
    print_status_graphics("ATA PIO Driver", true);
    
    /* Initialize UI */
    ui_init();
    ui_create_window(100, 100, 400, 300, "System Terminal", COLOR_BLACK);
    ui_create_window(550, 150, 300, 200, "OpenWare v0.1.1", COLOR_WHITE);
    
    /* Print ready message */
    vbe_print("\nOpenWare kernel initialized successfully!\n", COLOR_GREEN);
    vbe_print("Launching GUI System...\n", COLOR_WHITE);
    
    // For now, render once to show the windows
    ui_render();

    
    /* Initialize and run the shell */
    shell_init();
    shell_run();
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile("hlt");
    }
}
