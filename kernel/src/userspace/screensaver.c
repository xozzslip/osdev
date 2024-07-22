#include "../drivers/screen.h"
#include "../libk/assert.h"
#include "../libk/log.h"
#include "../libk/memory.h"
#include "../libk/process.h"
#include "../libk/sleep.h"
#include "../libk/string.h"
#include "../libk/syscall.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int pmain()
{
    uint8_t x = 0;
    uint8_t* first_ptr = 0;
    uint8_t* last_ptr = 0;
    void * some_buf = malloc(100);

    WindowResizedEvent resized = {0};
    WindowBuffer window = {0};

    for (;;) {
        KR kr = {
            .type = KR_RECV,
            .request.recv = {
                .path = "/proc/window/resized",
                .buf = &resized,
                .size = sizeof(resized),
                .nonblock = true,
            },
        };
        syscall(&kr);
        if (kr.response.recv.received > 0 && (window.width != resized.width || window.height != resized.height)) {
            klog(DEBUG, "window resized from %dX%d to %dX%d", window.width, window.height, resized.width, resized.height);
            if (window.buffer != NULL) {
                free(window.buffer);
            }
            window.width = resized.width;
            window.height = resized.height;
            uint32_t buffer_size = window.width * window.height * 2;
            klog(DEBUG, "allocating %d bytes for window buffer", buffer_size);
            window.buffer = malloc(buffer_size);
            KR kr = {
                .type = KR_SEND,
                .request.send = {
                    .path = "/proc/window/update",
                    .buf = &window,
                    .size = sizeof(window),
                }
            };
            syscall(&kr);
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



        kr = (KR){
            .type = KR_READ,
            .request.read = {
                .path = "/home/hello.txt",
                .buf = some_buf,
                .size = 10,
            },
        };
        syscall(&kr);

    }
}
