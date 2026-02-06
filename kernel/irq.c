#include "irq.h"
#include "pic.h"
#include "vbe.h"

static irq_handler_t irq_handlers[16] = {0};

void irq_init(void) {
    // Initialized by IDT
}

void irq_register_handler(int irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_handler(registers_t* regs) {
    int irq = regs->int_num - 32;

    if (irq >= 0 && irq < 16) {
        if (irq_handlers[irq] != 0) {
            irq_handlers[irq](regs);
        }
    }

    /* Send EOI to PIC */
    pic_send_eoi(irq);
}
