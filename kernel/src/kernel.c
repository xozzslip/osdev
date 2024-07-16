#include "drivers/keyboard.h"
#include "drivers/low_level.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "interrupts/interrupts.h"
#include "memory.h"
#include "libk/assert.h"
#include "libk/log.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

void dummy()
{
    dummy(); // do not enter there accidentelly
             // purpose of this function is to be forced to
             // call main instead of jumping into this file
}

u32 timer_called = 2;
u32 xui[1000];

void timer_callback(registers_t registers)
{
    return;
}

int main()
{
    init_serial();
    init_idt();
    init_timer(1);
    klog(INFO, "kernel is initializing...");
    register_irq_handler(0, timer_callback);
    register_irq_handler(1, keyboard_callback);
    asm volatile("sti");
    // char *video_memory = (char *) (0xb8000 + 2 * (14 * 80 + 2));
    // *video_memory = 'Z';
    setup_kernel_heap();
    for (int i = 0; i < 10; i++) {
        timer_called += 1;
    }
    uint8_t a = 10;
    uint8_t b = 11;
    uint8_t c = 12;
    int8_t d = -13;
    klog(INFO, "kernel was initialized successfully!");
    panic();

    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
