#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

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
} KR_WindowResizedEvent;


typedef struct {
    KR_WindowResizedEvent events[2];
    size_t count;
} KR_WindowResizedEvents;



typedef struct {
    uint8_t* new_ptr;
} KR_ChangeDataSegment;

typedef struct {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
} KR_RemapWindowBuffer;


typedef struct {
    uint32_t id;
    WindowBuffer window;

} Process;

#endif // PROCESS_H
