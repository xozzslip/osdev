#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

enum {
    WINDOW_RESIZED,
} typedef KernelEventType;

typedef struct {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
} WindowBuffer;

typedef struct {
    uint32_t width;
    uint32_t height;
} WindowResizedEvent;

typedef enum {
    KR_RECV_NONBLOCK,
    KR_OPEN,
    KR_READ,
    KR_WRITE,
    KR_SEEK,
    KR_ALLOCATE_MEMORY,
    KR_MAP_MEMORY,
    KR_WAIT,
} KR_Type;

typedef struct {
    const char* path;
    void *buf;
    size_t size;
} KR_RECV_NONBLOCK_Request;

typedef struct {
    uint32_t error;
} KR_RECV_NONBLOCK_Response;

typedef struct {
    KR_Type type;
    union {
        KR_RECV_NONBLOCK_Request recv_nonblock;
    } request;
    union {
        KR_RECV_NONBLOCK_Response recv_nonblock;
    } response;
} KR;


typedef struct {
    uint32_t id;
    WindowBuffer window;

} Process;

void syscall(KR* request);

#endif // PROCESS_H
