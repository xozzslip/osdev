#include "../../include/process.h"
#include "../../include/syscall.h"
#include "../libc/memory.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t my_glob = 123;
char *xui = "ololo";
uint32_t x[100];

void dummy()
{
    dummy();
}

int abc() {
    KR kr;
    kr = (KR){
        .type = KR_SENDINT,
        .request.sendint = {
            .path = "",
            .value = 11,
        },
    };
    syscall(&kr);
    kr = (KR){
        .type = KR_SENDINT,
        .request.sendint = {
            .path = "ololo",
            .value = "ololo"[0],
        },
    };
    syscall(&kr);

}

int _start()
{
    my_glob += 1;
    abc();
    for (;;) {
        continue;
    }
}



// int _start()
// {
//     uint8_t x = 0;
//     uint8_t* first_ptr = 0;
//     uint8_t* last_ptr = 0;

//     WindowResizedEvent resized = {0};
//     WindowBuffer window = {0};

//     for (;;) {
//         KR kr = {
//             .type = KR_RECV,
//             .request.recv = {
//                 .path = "/proc/window/resized",
//                 .buf = &resized,
//                 .size = sizeof(resized),
//                 .nonblock = true,
//             },
//         };
//         syscall(&kr);
//         if (kr.response.recv.received > 0 && (window.width != resized.width || window.height != resized.height)) {
//             if (window.buffer != NULL) {
//                 // free(window.buffer);
//             }
//             window.width = resized.width;
//             window.height = resized.height;
//             uint32_t buffer_size = window.width * window.height * 2;
//             window.buffer = malloc(buffer_size);
//             KR kr = {
//                 .type = KR_SEND,
//                 .request.send = {
//                     .path = "/proc/window/update",
//                     .buf = &window,
//                     .size = sizeof(window),
//                 }
//             };
//             syscall(&kr);
//         }

//         for (int row = 0; row < window.height; row++) {
//             for (int col = 0; col < window.width; col++) {
//                 uint32_t offset = col + row * window.width;
//                 char c = 'A' + (offset % 26);
//                 uint8_t* value = window.buffer + offset * 2;
//                 if (first_ptr == 0) {
//                     first_ptr = value;
//                 }
//                 value[0] = c;
//                 value[1] = x + offset;
//                 last_ptr = value;
//             }
//         }
//         x++;

//         for (int i = 0; i < 10000000;) {
//             i++;
//         }
//     }
// }
