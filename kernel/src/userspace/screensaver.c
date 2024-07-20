#include "../drivers/screen.h"
#include "../libk/log.h"
#include "../libk/memory.h"
#include "../libk/process.h"
#include "../libk/sleep.h"
#include "../libk/string.h"
#include "../libk/assert.h"
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

uint32_t mmap_window(WindowBuffer window)
{
    syscall(0, (uint32_t)window.buffer, window.width, window.height);
}

typedef enum {
    KR_WINDOW_RESIZE_POLL_EVENTS,
    KR_WINDOW_RESIZE_WAIT_EVENTS,
    KR_KEYBOARD_PRESS_POLL_EVENTS,
    KR_KEYBOARD_PRESS_WAIT_EVENTS,
    KR_CHANGE_DATA_SEGMENT,
    KR_REMAP_WINDOW_BUFFER,
} KernelRequestType;

typedef enum {
    KR_STATE_PENDING = 0,
    KR_STATE_ACCEPTED = 1,
    KR_STATE_SUCCEEDED = 2,
    KR_STATE_FAILED = 3,
} KernelRequestState;

typedef enum {
    KR_WAIT_NONE = 0,
    KR_WAIT_ANY = 1,
    KR_WAIT_ALL = 2,
} KernelRequestWaitType;

typedef struct {
    KernelRequestType type;
    uint8_t data[20];
    uint32_t id;
    KernelRequestState state;
    uint32_t error;
} KernelRequest;

void krs_send(KernelRequest* request, size_t count)
{
    _krs_send_and_maybe_wait(request, 1, KR_WAIT_NONE);
}

void kr_sendwait(KernelRequest* request)
{
    _krs_send_and_maybe_wait(request, 1, KR_WAIT_ALL);
}

void kr_wait(KernelRequest* request)
{
    _krs_send_and_maybe_wait(request, 1, KR_WAIT_ALL);
}

void krs_waitany(KernelRequest* requests, size_t count)
{
    _krs_send_and_maybe_wait(requests, count, KR_WAIT_ANY);
}

void _krs_send_or_wait(KernelRequest* request, size_t count, KernelRequestWaitType wait)
{
    uint32_t wait = 0;
    __asm__ volatile(
        "int $0x80"
        : // no return value
        : "a"((void*)request), // eax
        "b"(count) // ebx
        "c"(wait) // ecx
        // Tell the compiler to expect changes in memory and condition codes
        : "memory", "cc");
}

KR_WindowResizedEvents poll_window_resized_events() {
    KernelRequest request = {
        .type = KR_WINDOW_RESIZE_POLL_EVENTS,
    };
    kr_sendwait(&request);
    if (request.state != KR_STATE_SUCCEEDED) {
        klog(ERROR, "faild to poll window size event");
        panic();
    }
    KR_WindowResizedEvents events = *(KR_WindowResizedEvents *) request.data;
    return events;
}

WindowBuffer window;

int pmain()
{
    uint8_t x = 0;
    uint8_t* first_ptr = 0;
    uint8_t* last_ptr = 0;

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
        }
        ;
        break;
    default:
        break;
    }
}
