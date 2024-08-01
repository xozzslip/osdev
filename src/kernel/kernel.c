#include "../include/process.h"
#include "../include/string.h"
#include "../include/syscall.h"
#include "drivers/ata.h"
#include "drivers/fs.h"
#include "drivers/keyboard.h"
#include "drivers/low_level.h"
#include "drivers/rtc.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "interrupts/interrupts.h"
#include "libk/assert.h"
#include "libk/log.h"
#include "libk/memory.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

void dummy()
{
    dummy(); // do not enter there accidentelly
             // purpose of this function is to be forced to
             // call main instead of jumping into this file
}

uint32_t current_process_index;
Process* processes;
uint32_t current_timeslice;
#define MAX_PROCESSES 100

void timer_interrupt(registers_t* registers)
{
    current_timeslice++;
    // uint8_t* vga = (uint8_t*)VGA_TEXT_ADDRESS;
    // if (processes[0].window.buffer != NULL) {
    //     for (int j = 0; j < 25 * 80 * 2; j++) {
    //         vga[j] = processes[0].window.buffer[j];
    //     }
    // }
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

void drive_interrupt(registers_t* registers)
{
    return; // not used
}

void switch_task(registers_t* registers)
{
    processes[current_process_index].last_timeslice = current_timeslice;
    int32_t scheduled = current_process_index;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].running || processes[i].suspended) {
            continue;
        }
        if (processes[i].last_timeslice <= processes[scheduled].last_timeslice) {
            scheduled = i;
        }
    }
}

void syscall_handler(registers_t* registers)
{
    KR* kr = (KR*)registers->eax;
    switch (kr->type) {
    case KR_RECV:
        if (strcmp(kr->request.recv.path, "/proc/window/resized") == 0) {
            WindowResizedEvent event = {
                .width = 80,
                .height = 25,
            };
            memcpy(kr->request.recv.buf, &event, sizeof(event));
            kr->response.recv.error = 0;
            kr->response.recv.received = sizeof(WindowResizedEvent);
        }
        break;
    case KR_SEND:
        if (strcmp(kr->request.send.path, "/proc/window/update") == 0) {
            WindowBuffer window = { 0 };
            memcpy(&window, kr->request.send.buf, sizeof(window));
            processes[0].window = window;
        }
        break;
    case KR_READ:
        size_t size = 512;
        void* buf = malloc(size);
        // enqueue_drive_read(buf, size, 0);
        processes[0].suspended = true;
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
    setup_kernel_heap();
    init_idt();
    init_timer();
    init_drive();
    init_filesystem();
    register_interrupt_handler(32, timer_interrupt);
    register_interrupt_handler(33, keyboard_interrupt);
    register_interrupt_handler(46, drive_interrupt);
    register_interrupt_handler(128, syscall_handler);

    asm volatile("sti");

    processes = malloc(sizeof(Process) * 10);
    klog(INFO, "kernel was initialized successfully!");
    char* filepath = "/home/hello.txt";
    fs_read(filepath, 0, 10, NULL);

    /*
        start first process
    */
    // pmain();

    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
