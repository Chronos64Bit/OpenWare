#ifndef VBE_H
#define VBE_H

#include <stdint.h>

/**
 * VESA VBE Mode Info Block structure
 * As per VBE 2.0+ specifications
 */
typedef struct {
    uint16_t attributes;
    uint8_t window_a, window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a, segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;                 // Bytes per scanline

    uint16_t width, height;
    uint8_t w_char, y_char, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;

    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t reserved_mask, reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;           // Physical address of linear framebuffer (LFB)
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed)) vbe_mode_info_t;

/**
 * Common Colors (32-bit ARGB)
 */
#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0x00FFFFFF
#define COLOR_RED       0x00FF0000
#define COLOR_GREEN     0x0000FF00
#define COLOR_BLUE      0x000000FF
#define COLOR_YELLOW    0x00FFFF00
#define COLOR_CYAN      0x0000FFFF
#define COLOR_MAGENTA   0x00FF00FF
#define COLOR_GRAY      0x00808080

/**
 * Function prototypes
 */
void vbe_init(void);
void vbe_putpixel(int x, int y, uint32_t color);
void vbe_clear(uint32_t color);
void vbe_draw_rect(int x, int y, int w, int h, uint32_t color);
void vbe_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void vbe_enable_double_buffering(void);
void vbe_swap(void);
void vbe_draw_char(int x, int y, char c, uint32_t color);
void vbe_draw_string(int x, int y, const char* str, uint32_t color);
void vbe_print(const char* str, uint32_t color);




#endif // VBE_H
