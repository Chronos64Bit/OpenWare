#include "cmos.h"

/* Port I/O wrappers (inline assembly) */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static uint8_t get_update_in_progress_flag(void) {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

static uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

void cmos_init(void) {
    // Basic initialization if needed, mostly reading
}

void rtc_get_time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day, uint8_t* month, uint16_t* year) {
    uint8_t previous_sec;
    uint8_t registerB;

    // Wait until update is not in progress
    // If update is in progress, we might read inconsistent values
    while (get_update_in_progress_flag());
    
    *sec = get_rtc_register(0x00);
    *min = get_rtc_register(0x02);
    *hour = get_rtc_register(0x04);
    *day = get_rtc_register(0x07);
    *month = get_rtc_register(0x08);
    *year = get_rtc_register(0x09);

    do {
        previous_sec = *sec;

        while (get_update_in_progress_flag());
        
        *sec = get_rtc_register(0x00);
        *min = get_rtc_register(0x02);
        *hour = get_rtc_register(0x04);
        *day = get_rtc_register(0x07);
        *month = get_rtc_register(0x08);
        *year = get_rtc_register(0x09);
    } while (previous_sec != *sec);

    registerB = get_rtc_register(0x0B);

    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04)) {
        *sec = (*sec & 0x0F) + ((*sec / 16) * 10);
        *min = (*min & 0x0F) + ((*min / 16) * 10);
        *hour = ( (*hour & 0x0F) + (((*hour & 0x70) / 16) * 10) ) | (*hour & 0x80);
        *day = (*day & 0x0F) + ((*day / 16) * 10);
        *month = (*month & 0x0F) + ((*month / 16) * 10);
        *year = (*year & 0x0F) + ((*year / 16) * 10);
    }

    // Convert 12 hour to 24 hour if necessary
    if (!(registerB & 0x02) && (*hour & 0x80)) {
        *hour = ((*hour & 0x7F) + 12) % 24;
    }

    // Calculate full year
    // Heuristic: If year < 80, assume 20xx, else 19xx. 
    // Usually RTC only stores last 2 digits.
    // Let's assume 2000s for now, or use century register if available.
    *year += 2000;
}
