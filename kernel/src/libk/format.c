#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../drivers/screen.h"
#include "../drivers/serial.h"

void int_to_str(int32_t value, char* str)
{
    char* ptr = str;
    int temp;
    if (value < 0) {
        *ptr++ = '-';
        value = -value;
    }
    temp = value;
    do {
        ptr++;
        temp /= 10;
    } while (temp != 0);
    *ptr = '\0';
    do {
        *--ptr = (char)('0' + (value % 10));
        value /= 10;
    } while (value != 0);
}

void uint_to_str(uint32_t value, char* str)
{
    char* ptr = str;
    unsigned int temp;

    temp = value;
    do {
        ptr++;
        temp /= 10;
    } while (temp != 0);

    *ptr = '\0';

    do {
        *--ptr = (char)('0' + (value % 10));
        value /= 10;
    } while (value != 0);
}

void uint_to_hex(uint32_t value, char* str)
{
    char* ptr = str;
    int num_digits = sizeof(uint32_t) * 2;
    int i;

    ptr += num_digits;
    *ptr = '\0';

    for (i = 0; i < num_digits; i++) {
        int digit = value & 0xF;
        ptr--;
        *ptr = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        value >>= 4;
    }
}

enum {
    NORMAL,
    FORMAT_SPECIFIER,
} typedef State;

void format_and_write(bool (*write_byte)(char), char* format, va_list args)
{
    int i = 0;
    State state = NORMAL;
    bool write_is_ok = true;
    char* s = format;
    char c[20];

    uint32_t unsigned_value;
    int32_t signed_value;
    char* str_value;

    while (s[i] != '\0' && write_is_ok) {
        if (state == NORMAL) {
            if (s[i] == '%') {
                state = FORMAT_SPECIFIER;
            } else {
                char ch = s[i];
                write_is_ok = write_byte(ch);
            }
        } else if (state == FORMAT_SPECIFIER) {
            char* value; // stringified value
            for (int m = 0; m < 20; m++) {
                c[m] = 0;
            }

            switch (s[i]) {
            case 'u':
                unsigned_value = va_arg(args, uint32_t);
                uint_to_str(unsigned_value, c);
                value = c;
                break;
            case 'd':
                signed_value = va_arg(args, int32_t);
                int_to_str(signed_value, c);
                value = c;
                break;
            case 'x':
                unsigned_value = va_arg(args, uint32_t);
                uint_to_hex(unsigned_value, c);
                value = c;
                break;
            case 's':
                str_value = va_arg(args, char*);
                value = str_value;
                break;
            default:
                break;
            }
            while (*value != '\0' && write_is_ok) {
                write_is_ok = write_byte(*value);
                value++;
            }
            state = NORMAL;
        }
        i++;
    }
    va_end(args); // Cleaning up the list
}

bool _vga_text_write_byte(char c)
{
    char white_on_black = 0x0F;
    vga_text_write_byte(c, white_on_black);
    return true;
}

bool _serial_write_byte(char c)
{
    serial_write_byte(c);
    return true;
}

void format_and_write_to_vga_text(char* format, ...)
{
    va_list args;
    va_start(args, format);
    format_and_write(_vga_text_write_byte, format, args);
    va_end(args);
}

void format_and_write_to_serial_port(char* format, ...)
{
    va_list args;
    va_start(args, format);
    format_and_write(_serial_write_byte, format, args);
    va_end(args);
}
