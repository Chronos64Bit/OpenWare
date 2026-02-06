/**
 * OpenWare OS - Shell Implementation
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "shell.h"
#include "version.h"
#include "vga.h"
#include "keyboard.h"
#include "cmos.h"


#include "memory.h"

/* String utilities */
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

/* Command buffer */
static char input_buffer[SHELL_MAX_INPUT];
static size_t input_pos = 0;

/* Shell commands */
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_info(void);
static void cmd_echo(const char* args);
static void cmd_reboot(void);
static void cmd_apex(const char* args);
static void cmd_list(void);
static void cmd_view(const char* args);
static void cmd_mkdir(const char* args);
static void cmd_touch(const char* args);
static void cmd_palette(void);
static void cmd_date(void);
static void cmd_calc(const char* args);



/**
 * Print the shell prompt
 */
static void print_prompt(void) {
    vga_set_color(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts(SHELL_PROMPT);
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

/**
 * Process a command
 */
static void process_command(void) {
    /* Skip empty input */
    if (input_pos == 0) {
        return;
    }
    
    /* Null-terminate */
    input_buffer[input_pos] = '\0';
    
    /* Parse command */
    if (strcmp(input_buffer, "help") == 0) {
        cmd_help();
    } else if (strcmp(input_buffer, "clear") == 0 || strcmp(input_buffer, "cls") == 0) {
        cmd_clear();
    } else if (strcmp(input_buffer, "info") == 0 || strcmp(input_buffer, "sys") == 0) {
        cmd_info();
    } else if (strcmp(input_buffer, "reboot") == 0) {
        cmd_reboot();
    } else if (strncmp(input_buffer, "echo ", 5) == 0) {
        cmd_echo(input_buffer + 5);
    } else if (strcmp(input_buffer, "list") == 0 || strcmp(input_buffer, "ls") == 0) {
        cmd_list();
    } else if (strncmp(input_buffer, "view ", 5) == 0) {
        cmd_view(input_buffer + 5);
    } else if (strncmp(input_buffer, "mkdir ", 6) == 0) {
        cmd_mkdir(input_buffer + 6);
    } else if (strncmp(input_buffer, "touch ", 6) == 0) {
        cmd_touch(input_buffer + 6);
    } else if (strcmp(input_buffer, "palette") == 0) {
        cmd_palette();
    } else if (strcmp(input_buffer, "date") == 0) {
        cmd_date();
    } else if (strncmp(input_buffer, "calc ", 5) == 0) {
        cmd_calc(input_buffer + 5);
    } else if (strcmp(input_buffer, "echo") == 0) {
        vga_puts("\n");
    } else if (strncmp(input_buffer, "apex ", 5) == 0) {
        cmd_apex(input_buffer + 5);
    } else if (strcmp(input_buffer, "apex") == 0) {
        cmd_apex(NULL);
    } else if (strcmp(input_buffer, "mem") == 0) {
        // Simple memory test command
        vga_puts("Allocating 1024 bytes...\n");
        void* ptr = kmalloc(1024);
        if (ptr) {
            vga_puts("Allocation successful at: 0x");
            // print_hex((uint32_t)ptr); // Need hex print logic
            vga_puts("success\n");
            kfree(ptr);
            vga_puts("Freed.\n");
        } else {
            vga_puts("Allocation failed!\n");
        }
    } else {
        vga_set_color(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        vga_puts("Unknown command: ");
        vga_puts(input_buffer);
        vga_puts("\nType 'help' for available commands.\n");
        vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
    }
}

/**
 * Help command
 */
static void cmd_help(void) {
    vga_set_color(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    vga_puts("\nOpenWare Shell Commands:\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
    vga_puts("  help        - Show this help message\n");
    vga_puts("  clear, cls  - Clear the screen\n");
    vga_puts("  sys, info   - Show system information\n");
    vga_puts("  echo <text> - Print text to screen\n");
    vga_puts("  list        - List directory contents (ls)\n");
    vga_puts("  view <file> - View file contents (cat)\n");
    vga_puts("  mkdir <dir> - Create directory\n");
    vga_puts("  touch <fil> - Create empty file\n");
    vga_puts("  palette     - Show system colors\n");
    vga_puts("  date        - Show current date/time (UTC)\n");
    vga_puts("  calc <expr> - Simple calculator (e.g. 10 + 20)\n");
    vga_puts("  apex <cmd>  - Execute command with elevated privileges\n");
    vga_puts("  mem         - Test memory allocation\n");
    vga_puts("  reboot      - Reboot the system\n");
}

/* ... existing commands ... */

#include "memory.h"
#include "../fs/vfs.h"

/* ... existing code ... */

/**
 * List directory command
 */
static void cmd_list(void) {
    if (!fs_root) {
        vga_puts("[Error] No filesystem mounted.\n");
        return;
    }

    uint32_t i = 0;
    dirent_t* entry;
    
    vga_set_color(vga_entry_color(VGA_LIGHT_BLUE, VGA_BLACK));
    vga_puts("Directory listing for /:\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));

    while ((entry = vfs_readdir(fs_root, i)) != 0) {
        vga_puts("  ");
        vga_puts(entry->name);
        vga_puts("\n");
        i++;
    }
    
    if (i == 0) {
        vga_puts("  (empty)\n");
    }
}

/**
 * View file command
 */
static void cmd_view(const char* args) {
    if (args == NULL || *args == '\0') {
        vga_puts("Usage: view <filename>\n");
        return;
    }
    
    if (!fs_root) {
        vga_puts("[Error] No filesystem mounted.\n");
        return;
    }
    
    fs_node_t* file = vfs_finddir(fs_root, (char*)args);
    if (!file) {
        vga_puts("File not found: ");
        vga_puts(args);
        vga_puts("\n");
        return;
    }
    
    if ((file->flags & 0x7) == FS_DIRECTORY) {
        vga_puts("Is a directory.\n");
        kfree(file); // vfs_finddir allocates a new node
        return;
    }
    
    /* Read file content */
    uint32_t size = file->length;
    if (size == 0) {
        vga_puts("(empty file)\n");
        kfree(file);
        return;
    }
    
    // Limit view size for now
    if (size > 2048) {
        vga_puts("[Warning] File too large, showing first 2KB only.\n");
        size = 2048;
    }
    
    uint8_t* buffer = kmalloc(size + 1);
    uint32_t read = vfs_read(file, 0, size, buffer);
    buffer[read] = 0; // Null terminate for printing
    
    vga_puts((char*)buffer);
    vga_puts("\n");
    
    kfree(buffer);
    kfree(file);
}

/**
 * Make Directory command
 */
static void cmd_mkdir(const char* args) {
    if (args == NULL || *args == '\0') {
        vga_puts("Usage: mkdir <dirname>\n");
        return;
    }
    
    vga_puts("Creating directory: ");
    vga_puts(args);
    vga_puts("\n");
    /* TODO: VFS Implementation */
}

/**
 * Touch command
 */
static void cmd_touch(const char* args) {
    if (args == NULL || *args == '\0') {
        vga_puts("Usage: touch <filename>\n");
        return;
    }
    
    vga_puts("Creating file: ");
    vga_puts(args);
    vga_puts("\n");
    /* TODO: VFS Implementation */
}

/**
 * Palette command
 */
static void cmd_palette(void) {
    vga_puts("\nSystem Color Palette (VGA):\n");
    
    const char* color_names[] = {
        "Black", "Blue", "Green", "Cyan", "Red", "Magenta", "Brown", "LtGrey",
        "DkGrey", "LtBlue", "LtGreen", "LtCyan", "LtRed", "LtMagenta", "Yellow", "White"
    };

    for (int i = 0; i < 16; i++) {
        vga_set_color(vga_entry_color((uint8_t)i, VGA_BLACK));
        vga_puts("  [##] ");
        vga_puts(color_names[i]);
        if ((i + 1) % 4 == 0) vga_puts("\n");
    }
    vga_puts("\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

/**
 * Reboot command - triple fault method
 */
/**
 * Reboot command - triple fault method
 */
static void cmd_reboot(void) {
    vga_set_color(vga_entry_color(VGA_LIGHT_MAGENTA, VGA_BLACK));
    vga_puts("\nRebooting...\n");
    
    /* Disable interrupts */
    __asm__ volatile("cli");
    
    /* Clear IDT */
    uint8_t temp[6] = {0};
    __asm__ volatile("lidt %0" : : "m"(temp));
    
    /* Trigger triple fault */
    __asm__ volatile("int $3");
    
    /* Should never reach here */
    for(;;) __asm__ volatile("hlt");
}

/**
 * Clear screen command
 */
static void cmd_clear(void) {
    vga_clear();
}

/**
 * System info command
 */
static void cmd_info(void) {
    vga_set_color(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    vga_puts("\n=== OpenWare System Information ===\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
    
    vga_puts("  OS Name:     OpenWare\n");
    vga_puts("  Version:     " OS_VERSION_STRING " \"" OS_CODENAME "\"\n");
    vga_puts("  Build:       " STR(OS_BUILD) " (" OS_BUILD_DATE ")\n");
    vga_puts("  Vendor:      Ventryx Inc.\n");
    vga_puts("  Architecture: x86 (32-bit Protected Mode)\n");
    vga_puts("  Kernel:      Monolithic\n");
    vga_puts("  VGA Mode:    Text 80x25\n");
    
    vga_set_color(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    vga_puts("\n  (c) 2026 Ventryx Inc. All rights reserved.\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

/**
 * Echo command
 */
static void cmd_echo(const char* args) {
    vga_puts(args);
    vga_puts("\n");
}

/**
 * Apex (Sudo) command
 */
static void cmd_apex(const char* args) {
    if (args == NULL || *args == '\0') {
        vga_set_color(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        vga_puts("Usage: apex <command>\n");
        vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
        return;
    }
    
    vga_set_color(vga_entry_color(VGA_LIGHT_BROWN, VGA_BLACK));
    vga_puts("[APEX] Executing with elevated privileges: ");
    vga_puts(args);
    vga_puts("\n");
    
    vga_set_color(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts("Access Granted.\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

/**
 * Date command
 */
static void cmd_date(void) {
    uint8_t sec, min, hour, day, month;
    uint16_t year;
    
    rtc_get_time(&sec, &min, &hour, &day, &month, &year);
    
    // Simple printf-like output since we don't have printf
    vga_puts("Current Date/Time (UTC): ");
    
    // Helper to print 2 digits
    char buf[5];
    
    // Year
    int temp = year;
    buf[0] = (temp / 1000) + '0'; temp %= 1000;
    buf[1] = (temp / 100) + '0';  temp %= 100;
    buf[2] = (temp / 10) + '0';   temp %= 10;
    buf[3] = temp + '0';
    buf[4] = 0;
    vga_puts(buf);
    vga_puts("-");
    
    // Month
    buf[0] = (month / 10) + '0';
    buf[1] = (month % 10) + '0';
    buf[2] = 0;
    vga_puts(buf);
    vga_puts("-");
    
    // Day
    buf[0] = (day / 10) + '0';
    buf[1] = (day % 10) + '0';
    buf[2] = 0;
    vga_puts(buf);
    
    vga_puts(" ");
    
    // Hour
    buf[0] = (hour / 10) + '0';
    buf[1] = (hour % 10) + '0';
    buf[2] = 0;
    vga_puts(buf);
    vga_puts(":");
    
    // Min
    buf[0] = (min / 10) + '0';
    buf[1] = (min % 10) + '0';
    buf[2] = 0;
    vga_puts(buf);
    vga_puts(":");
    
    // Sec
    buf[0] = (sec / 10) + '0';
    buf[1] = (sec % 10) + '0';
    buf[2] = 0;
    vga_puts(buf);
    
    vga_puts("\n");
}


    vga_puts("\n");
}

/**
 * A simple atoi implementation
 */
static int atoi(const char* str) {
    int res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

/**
 * Calculator command
 */
static void cmd_calc(const char* args) {
    if (args == NULL || *args == '\0') {
        vga_puts("Usage: calc <num1> <op> <num2>\n");
        return;
    }
    
    // Parse first number
    int a = atoi(args);
    
    // Find operator
    while (*args >= '0' && *args <= '9') args++; // skip num1
    while (*args == ' ') args++; // skip spaces
    
    char op = *args;
    args++;
    
    // Find second number
    while (*args == ' ') args++; // skip spaces
    
    if (*args < '0' || *args > '9') {
        vga_puts("Error: Invalid expression\n");
        return;
    }
    
    int b = atoi(args);
    int result = 0;
    
    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/': 
            if (b == 0) {
                vga_puts("Error: Division by zero\n");
                return;
            }
            result = a / b; 
            break;
        default:
            vga_puts("Error: Unknown operator. Use +, -, *, /\n");
            return;
    }
    
    vga_puts("Result: ");
    
    // Print result (handle negative)
    if (result < 0) {
        vga_puts("-");
        result = -result;
    }
    
    // Print number 
    // (Simple recursive print for int)
    if (result == 0) {
        vga_puts("0");
    } else {
        char buf[12];
        int i = 0;
        while (result > 0) {
            buf[i++] = (result % 10) + '0';
            result /= 10;
        }
        // Reverse
        for (int j = 0; j < i / 2; j++) {
            char t = buf[j];
            buf[j] = buf[i - 1 - j];
            buf[i - 1 - j] = t;
        }
        buf[i] = 0;
        vga_puts(buf);
    }
    vga_puts("\n");
}


/**
 * Initialize the shell
 */
void shell_init(void) {
    input_pos = 0;
    
    vga_set_color(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    vga_puts("\nOpenWare Shell v0.1\n");
    vga_set_color(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    vga_puts("Type 'help' for available commands.\n\n");
    vga_set_color(vga_entry_color(VGA_WHITE, VGA_BLACK));
}

/**
 * Run the shell main loop
 */
void shell_run(void) {
    print_prompt();
    
    while (1) {
        char c = keyboard_getchar();
        
        switch (c) {
            case '\n':
                vga_putchar('\n');
                process_command();
                input_pos = 0;
                print_prompt();
                break;
                
            case '\b':
                if (input_pos > 0) {
                    input_pos--;
                    /* Move cursor back and erase character */
                    vga_puts("\b \b");
                }
                break;
                
            case '\t':
                /* Tab - could add auto-complete later */
                break;
                
            default:
                if (input_pos < SHELL_MAX_INPUT - 1 && c >= 32 && c < 127) {
                    input_buffer[input_pos++] = c;
                    vga_putchar(c);
                }
                break;
        }
    }
}
