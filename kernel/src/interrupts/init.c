#include "../types.h"

extern void isr0;
extern void isr1;
extern void isr2;
extern void isr3;
extern void isr4;
extern void isr5;
extern void isr6;
extern void isr7;
extern void isr8;
extern void isr9;
extern void isr10;
extern void isr11;
extern void isr12;
extern void isr13;
extern void isr14;
extern void isr15;
extern void isr16;
extern void isr17;
extern void isr18;
extern void isr19;
extern void isr20;
extern void isr21;
extern void isr22;
extern void isr23;
extern void isr24;
extern void isr25;
extern void isr26;
extern void isr27;
extern void isr28;
extern void isr29;
extern void isr30;
extern void isr31;

typedef struct {
    u16 base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
    u16 sel;                 // Kernel segment selector.
    u8  always0;             // This must always be zero.
    u8  flags;               // More flags. See documentation.
    u16 base_hi;             // The upper 16 bits of the address to jump to.

} idt_entry __attribute__((packed));

idt_entry idt[256];

idt_entry build_idt_entry(u8 num, u32 handler) {
    idt[num].base_lo = (u16) (handler & 0xFFFF);
    idt[num].base_hi = (u16) (handler >> 4);
    idt[num].sel = 0x08; // segment_code from gdt.asm

}

void init_idt() {


}
