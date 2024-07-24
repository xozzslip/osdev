#include "low_level.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// COPY pasted from https://wiki.osdev.org/CMOS

#define CURRENT_YEAR 2024 // Change this each year!

int century_register = 0x00; // Set by ACPI table parsing code if possible

unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;

enum {
    cmos_address = 0x70,
    cmos_data = 0x71
};

int get_update_in_progress_flag()
{
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}

unsigned char get_RTC_register(int reg)
{
    outb(cmos_address, reg);
    return inb(cmos_data);
}


void reread_rtc()
{
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (get_update_in_progress_flag()) {
        // Make sure an update isn't in progress
    }
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    if (century_register != 0) {
        century = get_RTC_register(century_register);
    }
    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag()) {
            // Make sure an update isn't in progress
        }
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
        if (century_register != 0) {
            century = get_RTC_register(century_register);
        }
    } while ((last_second != second) || (last_minute != minute) || (last_hour != hour) || (last_day != day) || (last_month != month) || (last_year != year) || (last_century != century));

    registerB = get_RTC_register(0x0B);

    // Convert BCD to binary values if necessary

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10)) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if (century_register != 0) {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    // Convert 12 hour clock to 24 hour clock if necessary

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year

    if (century_register != 0) {
        year += century * 100;
    } else {
        year += (CURRENT_YEAR / 100) * 100;
        if (year < CURRENT_YEAR)
            year += 100;
    }
}

// Utility functions
int isLeapYear(unsigned int year)
{
    if (year % 4 != 0)
        return 0;
    if (year % 100 != 0)
        return 1;
    if (year % 400 != 0)
        return 0;
    return 1;
}

int daysInMonth(unsigned int month, unsigned int year)
{
    switch (month) {
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return (isLeapYear(year) ? 29 : 28);
    default:
        return 31;
    }
}

// Convert date to Unix timestamp
uint32_t dateToUnixTimestamp()
{
    // Months are counted from January (1) to December (12), adjust if starting from zero
    // Days are assumed to be counted from 1 to 31

    // Epoch starts at 1970
    uint32_t timestamp = 0;
    unsigned int days = 0;

    // Count days for years since 1970
    for (unsigned int y = 1970; y < year; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }

    // Count days for past months of the current year
    for (unsigned char m = 1; m < month; m++) {
        days += daysInMonth(m, year);
    }

    // Current month's days (subtracting one as the current day should not be fully counted yet)
    days += day - 1;

    // Calculate total seconds
    timestamp = days * 86400ULL; // 86400 seconds per day
    timestamp += hour * 3600ULL; // 3600 seconds per hour
    timestamp += minute * 60ULL; // 60 seconds per minute
    timestamp += second; // seconds

    return timestamp;
}


uint32_t read_unix_time()
{
    reread_rtc();
    return dateToUnixTimestamp();
}
