#include "vbe.h"
#include "font.h"
#include "memory.h"
#include "types.h"


// Pointer to the mode info block stored by the bootloader at 0x5000
static vbe_mode_info_t* vbe_info = (vbe_mode_info_t*)0x5000;

static uint32_t* framebuffer = NULL;
static uint32_t screen_width = 0;
static uint32_t screen_height = 0;

static int term_x = 0;
static int term_y = 0;


void vbe_init(void) {
    if (vbe_info->framebuffer == 0) {
        // VBE not initialized by bootloader or failed
        return;
    }

    framebuffer = (uint32_t*)vbe_info->framebuffer;
    screen_width = vbe_info->width;
    screen_height = vbe_info->height;
    
    // Clear screen to Initial Blue (OpenWare branding)
    vbe_clear(0x00003366); 
    
    // Draw an welcome rectangle
    vbe_draw_rect(screen_width / 2 - 100, screen_height / 2 - 50, 200, 100, COLOR_WHITE);
}

void vbe_putpixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)screen_width || y < 0 || y >= (int)screen_height) {
        return;
    }

    // Assuming 32-bit color (RGBA) - 4 bytes per pixel
    // offset = y * pitch + x * 4
    // Since we use uint32_t* pointer, we divide pitch by 4
    uint32_t* target = backbuffer ? backbuffer : framebuffer;
    target[y * (vbe_info->pitch / 4) + x] = color;
}


void vbe_clear(uint32_t color) {
    uint32_t* target = backbuffer ? backbuffer : framebuffer;
    // Optimized fill for 32-bit
    for (uint32_t i = 0; i < screen_width * screen_height; i++) {
        target[i] = color;
    }
}


void vbe_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            vbe_putpixel(x + j, y + i, color);
        }
    }
}

static int abs(int x) {
    return (x < 0) ? -x : x;
}

void vbe_draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        vbe_putpixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static uint32_t* backbuffer = NULL;

void vbe_enable_double_buffering(void) {
    if (backbuffer) return;
    
    uint32_t size = screen_width * screen_height * sizeof(uint32_t);
    // Note: We need a kmalloc that can handle ~3MB if 1024x768x32.
    // Our heap in memory.h needs to be big enough.
    backbuffer = (uint32_t*)kmalloc(size);
    if (backbuffer) {
        kmemset(backbuffer, 0, size);
    }
}

void vbe_swap(void) {
    if (!backbuffer) return;
    kmemcpy(framebuffer, backbuffer, screen_width * screen_height * sizeof(uint32_t));
}

void vbe_draw_char(int x, int y, char c, uint32_t color) {
    if (c > 127) return;
    
    uint8_t* glyph = font8x8_basic[(int)c];
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (glyph[i] & (1 << (7 - j))) {
                vbe_putpixel(x + j, y + i, color);
            }
        }
    }
}

void vbe_draw_string(int x, int y, const char* str, uint32_t color) {
    int cur_x = x;
    while (*str) {
        if (*str == '\n') {
            y += 8;
            cur_x = x;
        } else {
            vbe_draw_char(cur_x, y, *str, color);
            cur_x += 8;
        }
        str++;
    }
}

void vbe_print(const char* str, uint32_t color) {
    while (*str) {
        if (*str == '\n') {
            term_x = 0;
            term_y += 8;
        } else if (*str == '\r') {
            term_x = 0;
        } else {
            vbe_draw_char(term_x, term_y, *str, color);
            term_x += 8;
            if (term_x >= (int)screen_width) {
                term_x = 0;
                term_y += 8;
            }
        }

        // Scrolling
        if (term_y >= (int)screen_height - 8) {
            uint32_t stride = screen_width * sizeof(uint32_t);
            uint32_t line_height = 8;
            uint32_t* target = backbuffer ? backbuffer : framebuffer;
            
            kmemcpy(target, target + (line_height * screen_width), (screen_height - line_height) * stride);
            kmemset(target + ((screen_height - line_height) * screen_width), 0, line_height * stride);
            term_y -= 8;
        }
        str++;
    }
    
    if (backbuffer) vbe_swap();
}


