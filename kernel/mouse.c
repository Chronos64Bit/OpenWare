#include "mouse.h"
#include "vbe.h"

#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_COMMAND_PORT 0x64

static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];
static int mouse_x = 0;
static int mouse_y = 0;

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(MOUSE_STATUS_PORT) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(MOUSE_STATUS_PORT) & 2) == 0) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xD4);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_DATA_PORT);
}

void mouse_handler(registers_t* regs) {
    (void)regs;
    
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if (!(status & 1) || !(status & 0x20)) return;

    mouse_packet[mouse_cycle++] = inb(MOUSE_DATA_PORT);

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        if (mouse_packet[0] & 0x80 || mouse_packet[0] & 0x40) return;

        int x_move = (int)mouse_packet[1];
        int y_move = (int)mouse_packet[2];

        if (mouse_packet[0] & 0x10) x_move -= 256;
        if (mouse_packet[0] & 0x20) y_move -= 256;

        mouse_x += x_move;
        mouse_y -= y_move; // Y is inverted in mouse protocol

        // Clamp to screen
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1024) mouse_x = 1024; // Needs dynamic check
        if (mouse_y > 768) mouse_y = 768;

        // Draw cursor (simple cross for now)
        // Note: Real cursor needs sprite and double buffering swap
        vbe_putpixel(mouse_x, mouse_y, COLOR_WHITE);
        vbe_putpixel(mouse_x+1, mouse_y, COLOR_WHITE);
        vbe_putpixel(mouse_x-1, mouse_y, COLOR_WHITE);
        vbe_putpixel(mouse_x, mouse_y+1, COLOR_WHITE);
        vbe_putpixel(mouse_x, mouse_y-1, COLOR_WHITE);
    }
}

void mouse_init(void) {
    uint8_t status;

    // Enable Auxiliary Device
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0xA8);

    // Enable interrupts
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x20);
    mouse_wait(0);
    status = (inb(MOUSE_DATA_PORT) | 2);
    mouse_wait(1);
    outb(MOUSE_COMMAND_PORT, 0x60);
    mouse_wait(1);
    outb(MOUSE_DATA_PORT, status);

    // Tell mouse to use default settings
    mouse_write(0xF6);
    mouse_read(); // Acknowledge

    // Enable mouse
    mouse_write(0xF4);
    mouse_read(); // Acknowledge

    // Register handler
    irq_register_handler(12, mouse_handler);
}
