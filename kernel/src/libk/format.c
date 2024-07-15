#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

void format_and_write(bool (*write_byte)(char), char* format, ...)
{
    va_list args;
    va_start(args, format);
    int i = 0;
    State state = NORMAL;
    bool write_is_ok = true;
    char* s = format;

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
            char c[20] = { 0 }; // big enough container for any stringified number
            switch (s[i]) {
            case 'u':
                uint_to_str(va_arg(args, uint32_t), c);
                value = c;
                break;
            case 'd':
                int_to_str(va_arg(args, int32_t), c);
                value = c;
                break;
            case 'x':
                uint_to_hex(va_arg(args, uint32_t), c);
                value = c;
                break;
            case 's':
                value = va_arg(args, char*);
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
