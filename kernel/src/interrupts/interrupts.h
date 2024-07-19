#if !defined(INTERRUPTS_H)
#define INTERRUPTS_H
#include "../types.h"
void init_idt();
struct registers_t {
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;
    u32 eip;
    u32 code_segment;
    u32 eflags;
};
typedef struct registers_t registers_t;
void register_interrupt_handler(int int_no, void (*handler)(registers_t registers));
#endif
