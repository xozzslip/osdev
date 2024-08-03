#include "../include/process.h"
#include "../include/string.h"
#include "../include/syscall.h"
#include "../include/utils.h"
#include "drivers/ata.h"
#include "drivers/fs.h"
#include "drivers/keyboard.h"
#include "drivers/low_level.h"
#include "drivers/rtc.h"
#include "drivers/screen.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "interrupts/interrupts.h"
#include "libk/assert.h"
#include "libk/log.h"
#include "libk/memory.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

void dummy()
{
    dummy(); // do not enter there accidentelly
             // purpose of this function is to be forced to
             // call main instead of jumping into this file
}

uint32_t current_process_index;
Process* processes;
uint32_t current_timeslice;
#define MAX_PROCESSES 100

void timer_interrupt(registers_t* registers)
{
    current_timeslice++;
    uint8_t* vga = (uint8_t*)VGA_TEXT_ADDRESS;
    if (processes[0].window.buffer != NULL) {
        for (int j = 0; j < 25 * 80 * 2; j++) {
            vga[j] = processes[0].window.buffer[j];
        }
    }
    return;
}

void drive_interrupt(registers_t* registers)
{
    return; // not used
}

void switch_task(registers_t* registers)
{
    processes[current_process_index].last_timeslice = current_timeslice;
    int32_t scheduled = current_process_index;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].running || processes[i].suspended) {
            continue;
        }
        if (processes[i].last_timeslice <= processes[scheduled].last_timeslice) {
            scheduled = i;
        }
    }
}

void syscall_handler(registers_t* registers)
{
    KR* kr = (KR*)registers->eax;
    // klog(DEBUG, "syscall occured type %d", kr->type);
    switch (kr->type) {
    case KR_RECV:
        // klog(DEBUG, "XUI! size=%d (0x%x) \"%s\"", kr->request.recv.size, kr->request.recv.size, kr->request.recv.path);
        if (strcmp(kr->request.recv.path, "/proc/window/resized") == 0) {
            WindowResizedEvent event = {
                .width = 80,
                .height = 25,
            };
            memcpy(kr->request.recv.buf, &event, sizeof(event));
            kr->response.recv.error = 0;
            kr->response.recv.received = sizeof(WindowResizedEvent);
        }
        break;
    case KR_SEND:
        if (strcmp(kr->request.send.path, "/proc/window/update") == 0) {
            WindowBuffer window = { 0 };
            memcpy(&window, kr->request.send.buf, sizeof(window));
            processes[0].window = window;
            klog(DEBUG, "window updated!");
        }
        break;
    case KR_READ:
        size_t size = 512;
        void* buf = malloc(size);
        // enqueue_drive_read(buf, size, 0);
        processes[0].suspended = true;
        break;
    case KR_MALLOC:
        kr->response.malloc.buf = malloc(kr->request.malloc.size);
        kr->response.malloc.error = 0;
        break;
    case KR_SENDINT:
        klog(DEBUG, "KR_SENDINT: %d", kr->request.sendint.value);
        break;
    default:
        break;
    }
}

struct ElfHeader {
    unsigned char e_ident[16]; // Magic number and other info
    uint16_t e_type; // Object file type
    uint16_t e_machine; // Architecture
    uint32_t e_version; // Object file version
    uint32_t e_entry; // Entry point virtual address
    uint32_t e_phoff; // Program header table file offset
    uint32_t e_shoff; // Section header table file offset
    uint32_t e_flags; // Processor-specific flags
    uint16_t e_ehsize; // ELF header size
    uint16_t e_phentsize; // Program header table entry size
    uint16_t e_phnum; // Program header table entry count
    uint16_t e_shentsize; // Section header table entry size
    uint16_t e_shnum; // Section header table entry count
    uint16_t e_shstrndx; // Section header string table index
} __attribute__((packed));
typedef struct ElfHeader ElfHeader;

struct ElfSectionHeader {
    uint32_t sh_name;      // Section name (index into the section header string table)
    uint32_t sh_type;      // Section type
    uint32_t sh_flags;     // Section flags
    uint32_t sh_addr;      // Address where section should be loaded
    uint32_t sh_offset;    // Offset of this section in the file image
    uint32_t sh_size;      // Size of this section in bytes
    uint32_t sh_link;      // Index of another section
    uint32_t sh_info;      // Additional section information
    uint32_t sh_addralign; // Section alignment
    uint32_t sh_entsize;   // Entry size if section holds a table
}__attribute__((packed));
typedef struct ElfSectionHeader ElfSectionHeader;

ElfHeader parse_elf_header(uint8_t* buf)
{
    assert (buf != NULL, "buffer for ELF copy was not provided");
    if (buf[0] != 0x7f || buf[1] != 'E' || buf[2] != 'L' || buf[3] != 'F') {
        klog(FATAL, "invalid ELF magic");
        panic();
    }
    ElfHeader header = {0};
    memcpy(&header, buf, sizeof(ElfHeader));
    if (header.e_ident[0] != 0x7f || header.e_ident[1] != 'E' || header.e_ident[2] != 'L' || header.e_ident[3] != 'F') {
        klog(FATAL, "ELF copy failed");
        panic();
    }
    return header;
}

int main()
{
    /*
        kernel initialization
    */

    init_serial();
    klog(INFO, "kernel is initializing...");
    setup_kernel_heap();
    init_idt();
    init_timer();
    init_drive();
    init_filesystem();
    register_interrupt_handler(32, timer_interrupt);
    register_interrupt_handler(33, keyboard_interrupt);
    register_interrupt_handler(46, drive_interrupt);
    register_interrupt_handler(128, syscall_handler);

    asm volatile("sti");

    processes = malloc(sizeof(Process) * 10);
    klog(INFO, "kernel was initialized successfully!");
    char* filepath = "/usr/screen.elf";
    uint8_t* buf = malloc(10000);
    int bytes = fs_read(filepath, 0, 10000, buf, false);
    klog(DEBUG, "%d", bytes);
    klog(DEBUG, "0x%x", buf);

    ElfHeader header = parse_elf_header(buf);
    klog(DEBUG, "0x%x", header.e_entry);
    for (int i = 0; i < header.e_shnum; i++) {
        ElfSectionHeader* section_header = (ElfSectionHeader*)(buf + header.e_shoff + i * header.e_shentsize);
        klog(DEBUG, "header type: %d", section_header->sh_type);
    }

    asm volatile("pushf");
    asm volatile("push $0x0008");
    asm volatile("push %0" ::"r"((uint32_t)buf + header.e_entry));
    asm volatile("iret" ::: "memory");

    klog(FATAL, "userspace program exited?");
    panic();
    /*
        start first process
    */
    // pmain();

    for (;;) {
        asm volatile("hlt");
    }
    return 0;
}
