/**
 * OpenWare OS - Keyboard Driver Header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef OPENWARE_KEYBOARD_H
#define OPENWARE_KEYBOARD_H

#include "../include/types.h"

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64
#define KEYBOARD_COMMAND_PORT   0x64

/* Keyboard status register bits */
#define KEYBOARD_OUTPUT_FULL    0x01
#define KEYBOARD_INPUT_FULL     0x02

/* Special key codes */
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_LCTRL       0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_LALT        0x38
#define KEY_CAPSLOCK    0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_NUMLOCK     0x45
#define KEY_SCROLLLOCK  0x46
#define KEY_UP          0x48
#define KEY_LEFT        0x4B
#define KEY_RIGHT       0x4D
#define KEY_DOWN        0x50

/* Key release offset */
#define KEY_RELEASE_OFFSET  0x80

/* Keyboard functions */
void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_key(void);
uint8_t keyboard_get_scancode(void);

/* IRQ1 handler (defined in isr.asm) */
extern void irq1(void);

#endif /* OPENWARE_KEYBOARD_H */
