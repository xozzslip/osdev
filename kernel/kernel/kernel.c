#include "../drivers/screen.h"
#include "../drivers/low_level.h"

void dummy() {
    dummy();  // do not enter there accidentelly
    // purpose of this function is to be forced to
    // call main instead of jumping into this file
}

int main() {
    // ospin_wait();
    char *video_memory = (char *) (0xb8000 + 2 * (14 * 80 + 2));
    *video_memory = 'Z';

    int offset = get_cursor_offset();
    for (int i = 0; i < 1000; i++) {
        offset++;
        set_cursor_offset(offset);
        spin_wait();
    }


    return 0;
}
