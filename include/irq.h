#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

/**
 * Register representation for interrupt handlers
 */
typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_num, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) registers_t;

typedef void (*irq_handler_t)(registers_t*);

void irq_init(void);
void irq_register_handler(int irq, irq_handler_t handler);
void irq_handler(registers_t* regs);

#endif // IRQ_H
