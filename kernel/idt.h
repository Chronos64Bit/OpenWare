/**
 * OpenWare OS - IDT (Interrupt Descriptor Table) Header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef OPENWARE_IDT_H
#define OPENWARE_IDT_H

#include "../include/types.h"

/* IDT entry structure */
typedef struct {
    uint16_t base_low;      /* Lower 16 bits of handler address */
    uint16_t selector;      /* Kernel segment selector */
    uint8_t  always0;       /* Always 0 */
    uint8_t  flags;         /* Type and attributes */
    uint16_t base_high;     /* Upper 16 bits of handler address */
} __attribute__((packed)) idt_entry_t;

/* IDT pointer structure */
typedef struct {
    uint16_t limit;         /* Size of IDT - 1 */
    uint32_t base;          /* Address of first IDT entry */
} __attribute__((packed)) idt_ptr_t;

/* IDT flags */
#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RING0      0x00
#define IDT_FLAG_RING3      0x60
#define IDT_FLAG_GATE_INT   0x0E    /* 32-bit interrupt gate */
#define IDT_FLAG_GATE_TRAP  0x0F    /* 32-bit trap gate */

/* Number of IDT entries */
#define IDT_ENTRIES         256

/* IDT functions */
void idt_init(void);
void idt_set_entry(uint8_t index, uint32_t base, uint16_t selector, uint8_t flags);

/* Exception ISR handlers (defined in assembly) */
extern void isr0(void);     /* Division by zero */
extern void isr1(void);     /* Debug */
extern void isr2(void);     /* NMI */
extern void isr3(void);     /* Breakpoint */
extern void isr4(void);     /* Overflow */
extern void isr5(void);     /* Bound range exceeded */
extern void isr6(void);     /* Invalid opcode */
extern void isr7(void);     /* Device not available */
extern void isr8(void);     /* Double fault */
extern void isr9(void);     /* Coprocessor segment overrun */
extern void isr10(void);    /* Invalid TSS */
extern void isr11(void);    /* Segment not present */
extern void isr12(void);    /* Stack segment fault */
extern void isr13(void);    /* General protection fault */
extern void isr14(void);    /* Page fault */
extern void isr15(void);    /* Reserved */
extern void isr16(void);    /* x87 floating point exception */
extern void isr17(void);    /* Alignment check */
extern void isr18(void);    /* Machine check */
extern void isr19(void);    /* SIMD floating point exception */

#endif /* OPENWARE_IDT_H */
