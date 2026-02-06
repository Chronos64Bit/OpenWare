/**
 * OpenWare OS - IDT (Interrupt Descriptor Table) Implementation
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "idt.h"
#include "pic.h"
#include "vga.h"


/* IDT entries */
static idt_entry_t idt_entries[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

/* External assembly function to load the IDT */
extern void idt_flush(uint32_t);

/* Exception messages */
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception"
};

/**
 * Set an IDT entry
 */
void idt_set_entry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[index].base_low = base & 0xFFFF;
    idt_entries[index].base_high = (base >> 16) & 0xFFFF;
    idt_entries[index].selector = selector;
    idt_entries[index].always0 = 0;
    idt_entries[index].flags = flags;
}

/**
 * Initialize the IDT
 */
void idt_init(void) {
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    /* Clear all IDT entries */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_entry(i, 0, 0, 0);
    }

    /* Set up exception handlers (ISRs 0-19) */
    idt_set_entry(0, (uint32_t)isr0, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(1, (uint32_t)isr1, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(2, (uint32_t)isr2, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(3, (uint32_t)isr3, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(4, (uint32_t)isr4, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(5, (uint32_t)isr5, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(6, (uint32_t)isr6, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(7, (uint32_t)isr7, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(8, (uint32_t)isr8, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(9, (uint32_t)isr9, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(10, (uint32_t)isr10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(11, (uint32_t)isr11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(12, (uint32_t)isr12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(13, (uint32_t)isr13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(14, (uint32_t)isr14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(15, (uint32_t)isr15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(16, (uint32_t)isr16, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(17, (uint32_t)isr17, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(18, (uint32_t)isr18, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(19, (uint32_t)isr19, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);

    /* Remap the PIC */
    pic_remap(0x20, 0x28);

    /* Set up IRQ handlers (mapped to 32-47) */
    idt_set_entry(32, (uint32_t)irq0, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(33, (uint32_t)irq1, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(34, (uint32_t)irq2, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(35, (uint32_t)irq3, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(36, (uint32_t)irq4, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(37, (uint32_t)irq5, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(38, (uint32_t)irq6, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(39, (uint32_t)irq7, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(40, (uint32_t)irq8, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(41, (uint32_t)irq9, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(42, (uint32_t)irq10, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(43, (uint32_t)irq11, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(44, (uint32_t)irq12, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(45, (uint32_t)irq13, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(46, (uint32_t)irq14, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);
    idt_set_entry(47, (uint32_t)irq15, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_INT);


    /* Load the IDT */
    idt_flush((uint32_t)&idt_ptr);
}

/**
 * Common exception handler called from assembly stubs
 */
void isr_handler(uint32_t int_num, uint32_t err_code) {
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_RED));
    vga_puts("\n*** EXCEPTION: ");
    
    if (int_num < 20) {
        vga_puts(exception_messages[int_num]);
    } else {
        vga_puts("Unknown Exception");
    }
    
    vga_puts(" ***\n");
    vga_set_color(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    
    /* Halt on exception */
    for(;;) {
        __asm__ volatile("cli; hlt");
    }
}
