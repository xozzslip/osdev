#include "../../include/syscall.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void* malloc(size_t size)
{
    KR kr = {
        .type = KR_MALLOC,
        .request.malloc = {
            .size = size,
        },
    };
    syscall(&kr);
    if (kr.response.malloc.error) {
        return NULL;
    }
    return kr.response.malloc.buf;
}
