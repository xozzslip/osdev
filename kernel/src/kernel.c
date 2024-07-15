#include "drivers/keyboard.h"
#include "drivers/low_level.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "interrupts/interrupts.h"
#include "memory.h"
#include "utils.h"
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
    init_idt();
    init_timer(1);
    init_serial();
    write_serial_str("Kernel is initializing...\n");
    register_irq_handler(0, timer_callback);
    register_irq_handler(1, keyboard_callback);
    asm volatile("sti");
    // char *video_memory = (char *) (0xb8000 + 2 * (14 * 80 + 2));
    // *video_memory = 'Z';
    int x = __LINE__;
    kprintf("line=%u\n", x);
    __ASSERT__(x > 0);
    __ASSERT__(x == 0);


    kprint("xli na\njui dva\nememabcde12345");
    kprint("000000\n");
    setup_kernel_heap();
    for (int i = 0; i < 10; i++) {
        kprintf("%d ", timer_called);
        timer_called += 1;
    }
    kprintf("\n");

    write_serial_str("Kernel was initialized successfully!\n");
    // int x = 0;
    // int y = 10 / x;

    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
