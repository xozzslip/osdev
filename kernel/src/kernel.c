#include "drivers/keyboard.h"
#include "drivers/low_level.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "interrupts/interrupts.h"
#include "libk/assert.h"
#include "libk/log.h"
#include "libk/memory.h"
#include "libk/process.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "userspace/screensaver.h"

void dummy()
{
    dummy(); // do not enter there accidentelly
             // purpose of this function is to be forced to
             // call main instead of jumping into this file
}

Process* processes;

void timer_callback(registers_t registers)
{
    uint8_t* vga = (uint8_t*)VGA_TEXT_ADDRESS;
    for (int j = 0; j < 25 * 80 * 2; j++) {
        vga[j] = processes[0].screen_buffer->buffer[j];
    }
    return;
}

int main()
{
    /*
        kernel initialization
    */
    init_serial();
    klog(INFO, "kernel is initializing...");
    init_idt();
    init_timer(1);
    register_irq_handler(0, timer_callback);
    register_irq_handler(1, keyboard_callback);
    asm volatile("sti");
    setup_kernel_heap();
    processes = malloc(sizeof(Process) * 10);
    klog(INFO, "kernel was initialized successfully!");

    /*
        start first process
    */
    ScreenBuffer* screen = malloc(sizeof(ScreenBuffer));
    screen->width = 80;
    screen->height = 25;
    screen->buffer = (uint8_t*)malloc(80 * 25 * 2);
    KernelEventHeader event = {
        .data = (uint8_t*)screen,
        .type = SCREEN_BUFFER_CHANGED,
    };
    processes[0].screen_buffer = screen;
    processes[0].id = 1;
    plistener(event);
    pmain();




    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
