#include "../types.h"
#include "../drivers/screen.h"
#include "../drivers/low_level.h"

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

struct idt_entry {
    u16 base_lo; // The lower 16 bits of the address to jump to when this interrupt fires.
    u16 sel;     // Kernel segment selector.
    u8  always0; // This must always be zero.
    u8  flags;   // More flags. See documentation.
    u16 base_hi; // The upper 16 bits of the address to jump to.
} __attribute__((packed));
typedef struct idt_entry idt_entry;

// TODO: make sure that this is on 8 byte boundary (check intel guide 6.10)
idt_entry idt[256];

idt_entry build_idt_entry(void (*handler)()) {
    u32 handler_address = (u32) handler;
    idt_entry e;
    e.base_lo = (u16) (handler_address & 0xFFFF);
    e.base_hi = (u16) ((handler_address >> 16) & 0xFFFF);
    e.sel = 0x08; // points to kernel "segment_code" (check gdt.asm)
    e.always0 = 0;
    e.flags = 0x8E;
    return e;
}

struct idt_entry_ptr {
    u16 limit; // The limit value is expressed in bytes and is added to the base address to get the address of the last valid byte.
    u32 base;  // The address of the first element in our idt_entry array.
}  __attribute__((packed));
typedef struct idt_entry_ptr idt_entry_ptr;

void setup_idt() {
    // zero all idt entries by my awesome version of memset
    // we need to do it, so all unused IDT entries have 0 in presence flag
    for (int i = 0; i < 256 * sizeof(idt_entry); i++) {
        ((u8 *)idt)[i] = (u8) 0;
    }

    idt[0]  = build_idt_entry(isr0);
    idt[1]  = build_idt_entry(isr1);
    idt[2]  = build_idt_entry(isr2);
    idt[3]  = build_idt_entry(isr3);
    idt[4]  = build_idt_entry(isr4);
    idt[5]  = build_idt_entry(isr5);
    idt[6]  = build_idt_entry(isr6);
    idt[7]  = build_idt_entry(isr7);
    idt[8]  = build_idt_entry(isr8);
    idt[9]  = build_idt_entry(isr9);
    idt[10] = build_idt_entry(isr10);
    idt[11] = build_idt_entry(isr11);
    idt[12] = build_idt_entry(isr12);
    idt[13] = build_idt_entry(isr13);
    idt[14] = build_idt_entry(isr14);
    idt[15] = build_idt_entry(isr15);
    idt[16] = build_idt_entry(isr16);
    idt[17] = build_idt_entry(isr17);
    idt[18] = build_idt_entry(isr18);
    idt[19] = build_idt_entry(isr19);
    idt[20] = build_idt_entry(isr20);
    idt[21] = build_idt_entry(isr21);
    idt[22] = build_idt_entry(isr22);
    idt[23] = build_idt_entry(isr23);
    idt[24] = build_idt_entry(isr24);
    idt[25] = build_idt_entry(isr25);
    idt[26] = build_idt_entry(isr26);
    idt[27] = build_idt_entry(isr27);
    idt[28] = build_idt_entry(isr28);
    idt[29] = build_idt_entry(isr29);
    idt[30] = build_idt_entry(isr30);
    idt[31] = build_idt_entry(isr31);

    // intialize special pointer structure
    idt_entry_ptr ptr;
    ptr.base = (u32) idt;
    ptr.limit = 256 * sizeof(idt_entry) - 1;

    // load IDT
    __asm__(
        "mov %0, %%eax\n\t"
        "lidt (%%eax)"
        :
        : "r"((u32) &ptr)
        : "eax"
    );
}

void isr_handler() {
    kprint("This is ISR_HANDLER");
    // spin_wait_milisecond();
    return;
}
