#include "../types.h"
#include "../interrupts/interrupts.h"
#include "low_level.h"
#include "screen.h"

typedef struct
{
    char ascii;
    bool is_pressed;  // or released
} keypress_packet;

char scancode_to_acsii(u8 scancode);


void keyboard_callback(registers_t register) {
    u8 scancode = inb(0x60);
    char symbol = scancode_to_acsii(scancode);
    if (symbol != 0) {
        char s[2] = {symbol, 0};
        kprint((char *)s);
    }
}


char scancode_to_acsii(u8 scancode) {
    switch (scancode) {
        case 0x2:
            return '1';
        case 0x3:
            return '2';
        case 0x4:
            return '3';
        case 0x5:
            return '4';
        case 0x6:
            return '5';
        case 0x7:
            return '6';
        case 0x8:
            return '7';
        case 0x9:
            return '8';
        case 0x0A:
            return '9';
        case 0x0B:
            return '0';
        case 0x0C:
            return '-';
        case 0x0D:
            return '+';
        case 0x10:
            return 'q';
        case 0x11:
            return 'w';
        case 0x12:
            return 'e';
        case 0x13:
            return 'r';
        case 0x14:
            return 't';
        case 0x15:
            return 'y';
        case 0x16:
            return 'u';
        case 0x17:
            return 'i';
        case 0x18:
            return 'o';
        case 0x19:
            return 'p';
		case 0x1A:
			return '[';
		case 0x1B:
			return ']';
		case 0x1C:
			return '\n';
		case 0x1E:
			return 'a';
		case 0x1F:
			return 's';
        case 0x20:
            return 'd';
        case 0x21:
            return 'f';
        case 0x22:
            return 'g';
        case 0x23:
            return 'h';
        case 0x24:
            return 'j';
        case 0x25:
            return 'k';
        case 0x26:
            return 'l';
        case 0x27:
            return ';';
        case 0x28:
            return '\'';
        case 0x29:
            return '`';
		case 0x2B:
			return '\\';
		case 0x2C:
			return 'z';
		case 0x2D:
			return 'x';
		case 0x2E:
			return 'c';
		case 0x2F:
			return 'v';
        case 0x30:
            return 'b';
        case 0x31:
            return 'n';
        case 0x32:
            return 'm';
        case 0x33:
            return ',';
        case 0x34:
            return '.';
        case 0x35:
            return '/';
        case 0x39:
            return ' ';
        default:
            return 0;
    }
}
