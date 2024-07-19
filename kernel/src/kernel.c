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
    if (processes[0].window.buffer != NULL) {
        for (int j = 0; j < 25 * 80 * 2; j++) {
            vga[j] = processes[0].window.buffer[j];
        }
    }

    return;
}

void syscall_handler(registers_t registers)
{
    switch (registers.eax) {
    case 0:
        processes[0].window.buffer = (uint8_t*)registers.ebx;
        processes[0].window.width = registers.ecx;
        processes[0].window.height = registers.edx;
        break;
    default:
        break;
    }
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
    register_interrupt_handler(32, timer_callback);
    register_interrupt_handler(33, keyboard_callback);
    register_interrupt_handler(128, syscall_handler);
    asm volatile("sti");
    setup_kernel_heap();
    processes = malloc(sizeof(Process) * 10);
    klog(INFO, "kernel was initialized successfully!");

    /*
        start first process
    */

    KernelEvent event = {
        .data.window_resized = {
            .width = 80,
            .height = 25 },
        .type = WINDOW_RESIZED,
    };
    processes[0].id = 1;
    plistener(event);
    pmain();

    asm volatile("int $128");

    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
