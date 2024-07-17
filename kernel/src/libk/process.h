#include <stdint.h>

enum {
    SCREEN_BUFFER_CHANGED,

} typedef KernelEventType;

struct {
    KernelEventType type;
    uint8_t *data;
} typedef KernelEventHeader;

enum {
    SCREEN_BUFFER_CHANGED,
} typedef ApplicationEventType;

struct {
    ApplicationEventHeader type;
    uint8_t *data;
} typedef ApplicationEventHeader;

struct {
    uint32_t width;
    uint32_t height;
    uint16_t *buffer;
} typedef ScreenBuffer;

struct {

} typedef ProcessInitParams;
