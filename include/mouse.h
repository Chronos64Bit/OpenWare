#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include "irq.h"

void mouse_init(void);
void mouse_handler(registers_t* regs);

#endif // MOUSE_H
