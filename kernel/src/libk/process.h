#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
} WindowBuffer;

typedef struct {
    uint32_t width;
    uint32_t height;
} WindowResizedEvent;

typedef struct {
    uint32_t id;
    WindowBuffer window;
    bool suspended;
} Process;

#endif // PROCESS_H
