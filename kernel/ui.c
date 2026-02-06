#include "ui.h"
#include "vbe.h"
#include "memory.h"

static window_t* windows[MAX_WINDOWS];
static int window_count = 0;

void ui_init(void) {
    for (int i = 0; i < MAX_WINDOWS; i++) windows[i] = NULL;
    window_count = 0;
}

int ui_create_window(int x, int y, int w, int h, const char* title, uint32_t color) {
    if (window_count >= MAX_WINDOWS) return -1;
    
    window_t* win = (window_t*)kmalloc(sizeof(window_t));
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;
    win->title = title;
    win->color = color;
    win->visible = true;
    win->dragging = false;
    
    windows[window_count++] = win;
    return window_count - 1;
}

static void draw_window(window_t* win) {
    if (!win->visible) return;
    
    // Draw Shadow (Simple offset)
    vbe_draw_rect(win->x + 4, win->y + 4, win->w, win->h, 0x00222222);
    
    // Draw Border & Background
    vbe_draw_rect(win->x, win->y, win->w, win->h, COLOR_GRAY);
    vbe_draw_rect(win->x + 2, win->y + 20, win->w - 4, win->h - 22, win->color);
    
    // Draw Title Bar
    vbe_draw_rect(win->x + 2, win->y + 2, win->w - 4, 18, 0x00336699);
    vbe_draw_string(win->x + 10, win->y + 5, win->title, COLOR_WHITE);
}

void ui_render(void) {
    vbe_clear(0x00003366); // Background
    
    // Draw all windows back-to-front
    for (int i = 0; i < window_count; i++) {
        if (windows[i]) draw_window(windows[i]);
    }
    
    vbe_swap();
}

void ui_handle_mouse(int x, int y, bool clicked) {
    (void)x; (void)y; (void)clicked;
    // Logic for dragging and clicking...
}
