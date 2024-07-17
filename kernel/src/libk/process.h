#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

enum {
    SCREEN_BUFFER_CHANGED,
} typedef KernelEventType;

typedef struct {
    KernelEventType type;
    uint8_t* data;
} KernelEventHeader;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* buffer;
} ScreenBuffer;

typedef struct {
    uint32_t id;
    ScreenBuffer *screen_buffer;
} Process;

#endif // PROCESS_H
