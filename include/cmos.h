#ifndef CMOS_H
#define CMOS_H

#include <stdint.h>
#include <stdbool.h>

/* CMOS registers */
#define CMOS_ADDRESS    0x70
#define CMOS_DATA       0x71

/* Function prototypes */
void cmos_init(void);
void rtc_get_time(uint8_t* sec, uint8_t* min, uint8_t* hour, uint8_t* day, uint8_t* month, uint16_t* year);

#endif // CMOS_H
