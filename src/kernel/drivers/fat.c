#include "../libk/assert.h"
#include "../libk/log.h"
#include "../libk/memory.h"
#include "low_level.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    READ,
    WRITE,
} RequestType;

typedef struct DriverRequest DriverRequest;

struct DriverRequest {
    void* buf;
    size_t size;
    RequestType type;

    DriverRequest* next;
};

DriverRequest* queue;

#define MAX_QUEUE_SIZE 100

void enqueue_drive_request(void* buf, size_t size, uint32_t lba, RequestType type)
{

    DriverRequest* cur = queue;
    DriverRequest* last = NULL;
    int queue_size = 0;
    while (cur != NULL) {
        last = cur;
        cur = cur->next;
        queue_size += 1;
    }
    if (queue_size >= MAX_QUEUE_SIZE) {
        klog(FATAL, "ATA driver queue size is too big, can't enqueue request");
        panic();
    }
    DriverRequest* request = (DriverRequest*)malloc(sizeof(DriverRequest));
    request->buf = buf;
    request->size = size;
    request->type = type;
    if (last != NULL) {
        last->next = request;
    } else {
        queue = request;
    }
}

void enqueue_drive_read(void* buf, size_t size, size_t lba)
{
    return enqueue_drive_request(buf, size, lba, READ);
}

void enqueue_drive_write(void* buf, size_t size, size_t lba)
{
    return enqueue_drive_request(buf, size, lba, WRITE);
}
