/**
 * OpenWare OS - VGA Text Mode Driver Header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef OPENWARE_VGA_H
#define OPENWARE_VGA_H

#include "../include/types.h"

/* VGA text mode constants */
#define VGA_WIDTH       80
#define VGA_HEIGHT      25
#define VGA_MEMORY      0xB8000

/* VGA colors */
typedef enum {
    VGA_BLACK           = 0,
    VGA_BLUE            = 1,
    VGA_GREEN           = 2,
    VGA_CYAN            = 3,
    VGA_RED             = 4,
    VGA_MAGENTA         = 5,
    VGA_BROWN           = 6,
    VGA_LIGHT_GREY      = 7,
    VGA_DARK_GREY       = 8,
    VGA_LIGHT_BLUE      = 9,
    VGA_LIGHT_GREEN     = 10,
    VGA_LIGHT_CYAN      = 11,
    VGA_LIGHT_RED       = 12,
    VGA_LIGHT_MAGENTA   = 13,
    VGA_LIGHT_BROWN     = 14,
    VGA_WHITE           = 15
} vga_color_t;

/**
 * Create a VGA color attribute byte
 */
static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

/**
 * Create a VGA character entry (character + color)
 */
static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* VGA driver functions */
void vga_init(void);
void vga_clear(void);
void vga_set_color(uint8_t color);
void vga_putchar(char c);
void vga_puts(const char* str);
void vga_put_at(char c, uint8_t x, uint8_t y);
void vga_set_cursor(uint8_t x, uint8_t y);

#endif /* OPENWARE_VGA_H */
