#include <stdint.h>

void spin_wait(uint32_t cycles)
{
    int x = 0;
    for (int i = 0; i < cycles; i++) {
        x += i;
    }
    return;
}
