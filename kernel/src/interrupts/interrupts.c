#include "../types.h"
#include "../drivers/screen.h"
#include "../drivers/low_level.h"
#include "interrupts.h"

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
extern void isr32(); // interrupts from PIC
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();

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
void (*irq_handlers[256])(registers_t);

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

#define PIC1_COMMAND 0x20
#define PIC2_COMMAND 0xA0
#define PIC1_DATA    0x21
#define PIC2_DATA    0xA1
void remap_pic() {
    port_byte_out(PIC1_COMMAND, 0x11);
    port_byte_out(PIC2_COMMAND, 0x11);
    port_byte_out(PIC1_DATA, 0x20);
    port_byte_out(PIC2_DATA, 0x28);
    port_byte_out(PIC1_DATA, 0x04);
    port_byte_out(PIC2_DATA, 0x02);
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);
    port_byte_out(PIC1_DATA, 0x0);
    port_byte_out(PIC2_DATA, 0x0);
}

void send_eoi_pic(u32 irq_no) {
    port_byte_out(0x20,0x20);
    if (irq_no >= 8) {
        port_byte_out(0xa0,0x20);
    }
}

void memset(void *ptr, u8 value, u32 size) {
    for (int i = 0; i < size; i++) {
        ((u8 *)ptr)[i] = value;
    }
}


void init_idt() {
    remap_pic();
    memset(&idt, 0, sizeof(idt_entry)*256);
    memset(&irq_handlers, 0, sizeof(void *));
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
    idt[32] = build_idt_entry(isr32);
    idt[33] = build_idt_entry(isr33);
    idt[34] = build_idt_entry(isr34);
    idt[35] = build_idt_entry(isr35);
    idt[36] = build_idt_entry(isr36);
    idt[37] = build_idt_entry(isr37);
    idt[38] = build_idt_entry(isr38);
    idt[39] = build_idt_entry(isr39);
    idt[40] = build_idt_entry(isr40);
    idt[41] = build_idt_entry(isr41);
    idt[42] = build_idt_entry(isr42);
    idt[43] = build_idt_entry(isr43);
    idt[44] = build_idt_entry(isr44);
    idt[45] = build_idt_entry(isr45);
    idt[46] = build_idt_entry(isr46);
    idt[47] = build_idt_entry(isr47);

    // intialize special pointer structure
    idt_entry_ptr ptr;
    ptr.base = (u32) &idt;
    ptr.limit = 256 * sizeof(idt_entry) - 1;

    __asm__ volatile("lidt %0" : : "m" (ptr));
}

void register_irq_handler(int irq_no, void(*handler)(registers_t registers)) {
    irq_handlers[irq_no] = handler;
}

void isr_handler(
    u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 int_no, u32 error_code, u32 eip, u32 code_segment, u32 eflags) {

    registers_t registers;
    registers.edi = edi;
    registers.esi = esi;
    registers.ebp = ebp;
    registers.esp = esp;
    registers.ebx = ebx;
    registers.edx = edx;
    registers.ecx = ecx;
    registers.eax = eax;
    registers.eip = eip;
    registers.code_segment = code_segment;
    registers.eflags = eflags;

    if (int_no >= 32) { // this is PIC interrupt
        u32 irq_no = int_no - 32;
        void (*handler)(registers_t) = irq_handlers[irq_no];
        if (handler != 0) {
            handler(registers);
        } else {
            kprint("unhandled IRQ: ");
            kprint("IRQ_NO=");
            kprint_u32(irq_no);
            kprint(" EIP=");
            kprint_u32(eip);
            kprint("\n");
        }
        send_eoi_pic(irq_no);
    } else {
        kprint("unhandled interrupt: ");
        kprint("INT_NO=");
        kprint_u32(int_no);
        kprint(" ERROR_CODE=");
        kprint_u32(error_code);
        kprint(" EIP=");
        kprint_u32(eip);
        kprint("\n");
    }
    return;
}
