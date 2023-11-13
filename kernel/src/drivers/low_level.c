#include "low_level.h"

unsigned char port_byte_in(unsigned short port) {
    unsigned char data = 0;
    __asm__(
        "mov %1, %%dx\n\t"
        "in %%dx, %%al\n\t"
        "mov %%al, %0"
        : "=r"(data)
        : "r"(port)
        : "al", "dx"
    );
    return data;
}

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__(
        "mov %1, %%dx\n\t"
        "mov %0, %%al\n\t"
        "out %%al, %%dx"
        :
        : "r"(data), "r"(port)
        : "al", "dx"
    );
}

unsigned char port_word_in(unsigned short port) {
    unsigned short data = 0;
    __asm__(
        "mov %1, %%dx\n\t"
        "in %%dx, %%ax\n\t"
        "mov %%ax, %0"
        : "=r"(data)
        : "r"(port)
        : "ax", "dx"
    );
    return data;
}

void port_word_out(unsigned short port, unsigned short data) {
    __asm__(
        "mov %1, %%dx\n\t"
        "mov %0, %%ax\n\t"
        "out %%ax, %%dx"
        :
        : "r"(data), "r"(port)
        : "ax", "dx"
    );
}

void spin_wait_second() {
    int x = 0;
        for (int i = 0; i < 100000000; i++) {
        x += i;
    }
    return;
}

void spin_wait_milisecond() {
    int x = 0;
        for (int i = 0; i < 100000; i++) {
        x += i;
    }
    return;
}


void empty_func() {
    int a = 10;
    return;
}
