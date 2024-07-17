#include "../libk/memory.h"
#include "../libk/process.h"
#include "../libk/string.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

ScreenBuffer screen;

int main()
{
    uint8_t x = 0;
    for (;;) {
        for (int row = 0; row < screen.height; row++) {
            for (int col = 0; col < screen.width; col++) {
                uint32_t offset = col + row * screen.width;
                char c = 'A' + offset % 28;
                uint16_t* value = screen.buffer + offset;
                uint8_t* letter = value;
                uint8_t* color = value + 1;
                *letter = c;
                *color = x+offset;
            }
        }
        x++;
    }
}

void listener(KernelEventHeader event)
{
    switch (event.type) {
    case SCREEN_BUFFER_CHANGED:
        screen = *(ScreenBuffer*)event.data;
        break;
    default:
        break;
    }
}
