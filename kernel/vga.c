/**
 * OpenWare OS - VGA Text Mode Driver
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "vga.h"

/* Static variables */
static uint16_t* vga_buffer = (uint16_t*)VGA_MEMORY;
static uint8_t vga_row = 0;
static uint8_t vga_col = 0;
static uint8_t vga_color = 0;

/**
 * Initialize VGA text mode
 */
void vga_init(void) {
    vga_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
}

/**
 * Clear the screen
 */
void vga_clear(void) {
    uint16_t blank = vga_entry(' ', vga_color);
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = blank;
        }
    }
    
    vga_row = 0;
    vga_col = 0;
}

/**
 * Set current text color
 */
void vga_set_color(uint8_t color) {
    vga_color = color;
}

/**
 * Scroll the screen up by one line
 */
static void vga_scroll(void) {
    uint16_t blank = vga_entry(' ', vga_color);
    
    /* Move all lines up by one */
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    /* Clear the last line */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    
    vga_row = VGA_HEIGHT - 1;
}

/**
 * Print a single character
 */
void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
        if (vga_row >= VGA_HEIGHT) {
            vga_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        vga_col = 0;
        return;
    }
    
    if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
        } else if (vga_row > 0) {
            vga_row--;
            vga_col = VGA_WIDTH - 1;
        }
        return;
    }
    
    if (c == '\t') {
        vga_col = (vga_col + 4) & ~3;  /* Align to 4 spaces */
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
            if (vga_row >= VGA_HEIGHT) {
                vga_scroll();
            }
        }
        return;
    }
    
    vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_color);
    vga_col++;
    
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
        if (vga_row >= VGA_HEIGHT) {
            vga_scroll();
        }
    }
}

/**
 * Print a null-terminated string
 */
void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/**
 * Put a character at a specific position
 */
void vga_put_at(char c, uint8_t x, uint8_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    vga_buffer[y * VGA_WIDTH + x] = vga_entry(c, vga_color);
}

/**
 * Set cursor position
 */
void vga_set_cursor(uint8_t x, uint8_t y) {
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;
    vga_col = x;
    vga_row = y;
}
