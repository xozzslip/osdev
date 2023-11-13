#include "drivers/screen.h"
#include "drivers/low_level.h"

void dummy() {
    dummy();  // do not enter there accidentelly
    // purpose of this function is to be forced to
    // call main instead of jumping into this file
}

int main() {
    // ospin_wait();
    char *video_memory = (char *) (0xb8000 + 2 * (14 * 80 + 2));
    *video_memory = 'Z';
    kprint("xli na\njui dva\nememabcde12345");
    kprint("000000");
    return 0;
}
