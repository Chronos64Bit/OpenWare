#include "keyboard.h"
#include "idt.h"
#include "irq.h"
#include "vbe.h"


/* I/O port access */
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* Keyboard state */
static bool shift_pressed = false;
static bool caps_lock = false;
static volatile char last_char = 0;
static volatile bool key_ready = false;

/* US keyboard layout - lowercase */
static const char scancode_to_ascii[] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',  0,   ' '
};

/* US keyboard layout - uppercase/shifted */
static const char scancode_to_ascii_shift[] = {
    0,    0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*',  0,   ' '
};

/**
 * Keyboard IRQ handler - called from assembly
 */
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    /* Check for key release */
    if (scancode & KEY_RELEASE_OFFSET) {
        scancode &= ~KEY_RELEASE_OFFSET;
        if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
            shift_pressed = false;
        }
        return;
    }
    
    /* Handle special keys */
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            shift_pressed = true;
            return;
        case KEY_CAPSLOCK:
            caps_lock = !caps_lock;
            return;
        case KEY_LCTRL:
        case KEY_LALT:
            return;
    }
    
    /* Convert scancode to ASCII */
    if (scancode < sizeof(scancode_to_ascii)) {
        char c;
        bool use_upper = shift_pressed;
        
        /* Caps lock affects only letters */
        if (caps_lock && scancode >= 0x10 && scancode <= 0x32) {
            use_upper = !use_upper;
        }
        
        if (use_upper) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
        
        if (c != 0) {
            last_char = c;
            key_ready = true;
        }
    }
}

/**
 * Send End Of Interrupt to PIC
 */
static void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(0xA0, 0x20);  /* Slave PIC */
    }
    outb(0x20, 0x20);      /* Master PIC */
}

/**
 * Keyboard IRQ handler - called from central irq dispatcher
 */
void keyboard_irq_handler(registers_t* regs) {
    (void)regs; // Unused
    keyboard_handler();
}


// Local PIC remapping logic removed, now handled by idt_init/pic_remap


/**
 * Initialize keyboard driver
 */
void keyboard_init(void) {
    /* Register IRQ1 handler */
    irq_register_handler(1, keyboard_irq_handler);
    
    /* Enable IRQ1 in Master PIC mask (Keyboard) and IRQ12 in Slave PIC (Mouse) */
    /* Keyboard is already likely enabled, but let's be explicit later in pic_init */
    
    /* Enable interrupts locally */
    __asm__ volatile("sti");
}


/**
 * Check if a key is available
 */
bool keyboard_has_key(void) {
    return key_ready;
}

/**
 * Get a character (blocking)
 */
char keyboard_getchar(void) {
    while (!key_ready) {
        __asm__ volatile("hlt");
    }
    key_ready = false;
    return last_char;
}

/**
 * Get raw scancode
 */
uint8_t keyboard_get_scancode(void) {
    while (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_OUTPUT_FULL)) {
        __asm__ volatile("hlt");
    }
    return inb(KEYBOARD_DATA_PORT);
}
