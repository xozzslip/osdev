#include "../libk/memory.h"
#include "../libk/process.h"
#include "../libk/string.h"
#include "../libk/log.h"
#include "../libk/sleep.h"
#include "../drivers/screen.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

ScreenBuffer screen;

int pmain()
{
    uint8_t x = 0;
    uint8_t *first_ptr = 0;
    uint8_t *last_ptr = 0;

    for (;;) {
        for (int row = 0; row < screen.height; row++) {
            for (int col = 0; col < screen.width; col++) {
                uint32_t offset = col + row * screen.width;
                char c = 'A' + (offset % 26);
                uint8_t* value = screen.buffer + offset * 2;
                if (first_ptr == 0) {
                    first_ptr = value;
                }
                value[0] = c;
                value[1] = x + offset;
                last_ptr = value;
            }
        }
        x++;
        spin_wait(10000000);


    }

}

void plistener(KernelEventHeader event)
{
    switch (event.type) {
    case SCREEN_BUFFER_CHANGED:
        screen = *(ScreenBuffer*)event.data;
        break;
    default:
        break;
    }
}
