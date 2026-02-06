#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>
#include "vbe.h"

#define MAX_WINDOWS 32

typedef struct {
    int x, y, w, h;
    uint32_t color;
    const char* title;
    bool visible;
    bool dragging;
    // Buffer for window content (optional for now, can draw direct)
} window_t;

void ui_init(void);
void ui_render(void);
int ui_create_window(int x, int y, int w, int h, const char* title, uint32_t color);
void ui_handle_mouse(int x, int y, bool clicked);

#endif // UI_H
