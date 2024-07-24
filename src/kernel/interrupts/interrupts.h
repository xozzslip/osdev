#if !defined(INTERRUPTS_H)
#define INTERRUPTS_H
#include <stdint.h>
void init_idt();
struct registers_t {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t int_no;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};
typedef struct registers_t registers_t;
void register_interrupt_handler(int int_no, void (*handler)(registers_t *registers));
#endif
