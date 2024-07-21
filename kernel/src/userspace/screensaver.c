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

typedef enum {
    KR_OPEN,
    KR_READ,
    KR_WRITE,
    KR_SEEK,
    KR_ALLOCATE_MEMORY,
    KR_MAP_MEMORY,
    KR_WAIT,
} KR_Type;

typedef struct {
    KR_Type type;
    union
    {
        KR_OPEN_Request open_request;
        KR_OPEN_Response open_response;
    } data;

} KR;

typedef struct {
    const char *path;
} KR_OPEN_Request;

typedef struct {
    uint32_t error;
} KR_OPEN_Response;


void syscall(KR* request)
{
    __asm__ volatile(
    "int $0x80"
    : // no return value
    : "a"((void*)request), // eax
    "b"(1) // ebx
    // Tell the compiler to expect changes in memory and condition codes
    : "memory", "cc");
}


uint32_t open(KR_OPEN_Request req)
{
    KR request = {0};
    request.type = KR_OPEN;
    request.data.open_request = req;
    syscall(&request);
    return request.data.open_response;
}

WindowBuffer window;

int pmain()
{
    uint8_t x = 0;
    uint8_t* first_ptr = 0;
    uint8_t* last_ptr = 0;



    for (;;) {


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
