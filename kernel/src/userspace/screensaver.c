#include "../drivers/screen.h"
#include "../libk/assert.h"
#include "../libk/log.h"
#include "../libk/memory.h"
#include "../libk/process.h"
#include "../libk/sleep.h"
#include "../libk/string.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t syscall(
    uint32_t syscall_no,
    uint32_t param1,
    uint32_t param2,
    uint32_t param3)
{
    uint32_t ret; // To store the return value of the syscall
    __asm__ volatile(
        "int $0x80" // Interrupt instruction to invoke the system call
        : "=a"(ret) // Output: eax will hold the return value after syscall
        : "a"(syscall_no), // Input: eax should hold the syscall number
        "b"(param1), // ebx should hold the first argument
        "c"(param2), // ecx should hold the second argument
        "d"(param3) // edx should hold the third argument
        : "memory", "cc" // Tell the compiler to expect changes in memory and condition codes
    );
    return ret;
}

typedef enum {
    KR_RECV_NONBLOCK,
    KR_WRITE,
    KR_SEEK,
    KR_ALLOCATE_MEMORY,
    KR_MAP_MEMORY,
    KR_WAIT,
} KernelRequestType;

int32_t recv_nonblock(const char* path, void* buf, size_t size)
{
    return syscall(
        KR_RECV_NONBLOCK,
        (uint32_t)path,
        (uint32_t)buf,
        (uint32_t)size);
}

WindowBuffer window;

int pmain()
{
    uint8_t x = 0;
    uint8_t* first_ptr = 0;
    uint8_t* last_ptr = 0;

    int32_t window_fd = open("/dev/window");
    if (window_fd <= 0) {
        return -1;
    }

    for (;;) {
        KR_WindowResizedEvents events = poll_window_resized_events();
        if (events.count > 0) {
            KR_WindowResizedEvent resize = events.events[events.count - 1];
            window.width = resize.width;
            window.height = resize.height;
            window.buffer = (uint8_t*)malloc(window.width * window.height * 2);
            mmap_window(window)
        }

        for (int row = 0; row < window.height; row++) {
            for (int col = 0; col < window.width; col++) {
                uint32_t offset = col + row * window.width;
                char c = 'A' + (offset % 26);
                uint8_t* value = window.buffer + offset * 2;
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

void plistener(KernelEvent event)
{
    switch (event.type) {
    case WINDOW_RESIZED:
        if (window.buffer != NULL) {
            free(window.buffer);
        };
        break;
    default:
        break;
    }
}
