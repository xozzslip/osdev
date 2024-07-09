#include "drivers/screen.h"
#include "drivers/low_level.h"
#include "drivers/timer.h"
#include "drivers/keyboard.h"
#include "interrupts/interrupts.h"

void dummy() {
    dummy();  // do not enter there accidentelly
    // purpose of this function is to be forced to
    // call main instead of jumping into this file
}

u32 timer_called = 0 ;

void timer_callback(registers_t registers) {
    return;
}


int main() {
    // ospin_wait();
    int x = 10;
    kprint_u32(x);
    init_idt();
    init_timer(1);
    register_irq_handler(0, timer_callback);

    register_irq_handler(1, keyboard_callback);
    asm volatile("sti");
    // char *video_memory = (char *) (0xb8000 + 2 * (14 * 80 + 2));
    // *video_memory = 'Z';
    kprint("xli na\njui dva\nememabcde12345");
    kprint("000000\n");

    for(;;) {
        asm("hlt");
    }
    return 0;
}
