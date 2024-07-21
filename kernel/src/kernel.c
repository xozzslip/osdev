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
#include "libk/string.h"
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

void memcpy(void* dest, void* src, size_t n)
{
    char* d = (char*)dest;
    char* s = (char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void syscall_handler(registers_t registers)
{
    KR* kr = (KR*)registers.eax;


    switch (kr->type) {
    case KR_RECV_NONBLOCK:
        klog(INFO, "KR_RECV_NONBLOCK syscall received");
        if (streq(kr->request.recv_nonblock.path, "/proc/window/resized")) {
            WindowResizedEvent event = {
                .width = 80,
                .height = 25,
            };
            memcpy(kr->request.recv_nonblock.buf, &event, sizeof(event));
            kr->response.recv_nonblock.error = 0;
            kr->response.recv_nonblock.received = sizeof(WindowResizedEvent);
        }
        break;
    case KR_SEND:
        klog(INFO, "KR_SEND syscall received");
        if (streq(kr->request.send.path, "/proc/window/update")) {
            WindowBuffer window = {0};
            memcpy(&window, kr->request.send.buf, sizeof(window));
            processes[0].window = window;
        }
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

    pmain();

    asm volatile("int $128");

    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
