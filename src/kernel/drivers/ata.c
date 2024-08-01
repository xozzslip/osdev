#include "../libk/assert.h"
#include "../libk/log.h"
#include "../libk/memory.h"
#include "low_level.h"

#include <stddef.h>
#include <stdint.h>

#define DATA_REGISTER 0x1F0
#define ERROR_REGISTER 0x1F1
#define FEATURES_REGISTER 0x1F1
#define SECTOR_COUNT_REGISTER 0x1F2
#define LBA_LO_REGISTER 0x1F3
#define LBA_MID_REGISTER 0x1F4
#define LBA_HI_REGISTER 0x1F5
#define DRIVE_HEAD_REGISTER 0x1F6
#define COMMAND_REGISTER 0x1F7
#define STATUS_REGISTER 0x1F7

#define DEVICE_CONTROL_REGISTER 0x3F6
#define ALTERNATE_STATUS_REGISTER 0x3F6

typedef struct {
    bool err; // 0	ERR	Indicates an error occurred. Send a new command to clear it (or nuke it with a Software Reset).
    bool idx; // 1	IDX	Index. Always set to zero.
    bool corr; // 2	CORR	Corrected data. Always set to zero.
    bool drq; // 3	DRQ	Set when the drive has PIO data to transfer, or is ready to accept PIO data.
    bool srv; // 4	SRV	Overlapped Mode Service Request.
    bool df; // 5	DF	Drive Fault Error (does not set ERR).
    bool rdy; // 6	RDY	Bit is clear when drive is spun down, or after an error. Set otherwise.
    bool bsy; // 7	BSY	Indicates the drive is preparing to send/receive data (wait for it to clear). In case of 'hang' (it never clears), do a software reset.
} Status;

Status parse_status(uint8_t status)
{
    Status res = { 0 };
    res.err = status & 1;
    res.idx = status & 2;
    res.corr = status & 4;
    res.drq = status & 8;
    res.srv = status & 16;
    res.df = status & 32;
    res.rdy = status & 64;
    res.bsy = status & 128;
    return res;
}

typedef enum {
    UNKNOWN,
    READ,
    WRITE,
} RequestType;

typedef struct {
    RequestType type;
    uint32_t lba;
    uint32_t sectors_requested;
    uint32_t sectors_processed;
    void* buf;
    bool running;
} Request;

Request current;

void drive_send_read_command(uint32_t lba, uint32_t sectors_count, void* buf)
{
    assert(lba < (1 << 28), "lba must be 28 bits");
    assert(sectors_count <= 256, "too much sectors to read");
    assert(!current.running, "some request is already running");
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert(!status.err && !status.df, "drive error");
    assert(!status.bsy, "drive status is busy");
    uint8_t sectors_count_adjusted = sectors_count;
    if (sectors_count == 256) {
        sectors_count_adjusted = 0; // 0 is special value to read 256 sectors
    }

    outb(DRIVE_HEAD_REGISTER, 0xE0 | ((lba >> 24) & 0x0F)); // Send 0xE0 for the "master" or 0xF0 for the "slave", ORed with the highest 4 bits of the LBA
    outb(SECTOR_COUNT_REGISTER, sectors_count_adjusted); // Send the sector count
    outb(LBA_LO_REGISTER, (uint8_t)lba); // Send the low 8 bits of lba
    outb(LBA_MID_REGISTER, (uint8_t)(lba >> 8));
    outb(LBA_HI_REGISTER, (uint8_t)(lba >> 16));
    outb(COMMAND_REGISTER, 0x20); // Send the "READ SECTORS" command

    current.lba = lba;
    current.sectors_requested = sectors_count;
    current.sectors_processed = 0;
    current.type = READ;
    current.running = true;
    current.buf = buf;

    klog(DEBUG, "sending read sectors command to drive: lba=%d sectors=%d buf=0x%x", lba, sectors_count, buf);
}

bool is_drive_ready()
{
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert(!status.err && !status.df, "drive error");
    return !status.bsy && status.drq;
}

void drive_receive_sector()
{
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert(!status.err && !status.df, "drive error");
    assert(!status.bsy && status.drq, "data not ready");
    assert(current.running, "request must be in progress");
    assert(current.type == READ, "read request must be in progress");
    void* buf = current.buf + current.sectors_processed * 512;
    // Clear direction flag to increment EDI after each read
    asm volatile("cld" : : : "cc");
    // Read data from DATA_REGISTER port to buffer
    asm volatile(
        "rep insw" // Repeat input from port
        : /* no outputs */
        : "d"(DATA_REGISTER), // EDX = port
        "D"(buf), // EDI = buffer
        "c"(256) // ECX = 16-bit words count
        : "memory", "cc" // Clobbered: Memory and condition codes
    );

    current.sectors_processed += 1;

    klog(DEBUG, "sector %d/%d received to buf=0x%x", current.sectors_processed, current.sectors_requested, current.buf);

    if (current.sectors_processed == current.sectors_requested) {
        current = (Request) { 0 };
        current.running = false; // it is already set, but anyway
    }
}

void drive_read_blocking(uint32_t lba, uint32_t sectors_count, void* buf)
{
    drive_send_read_command(lba, sectors_count, buf);

    for (int i = 0; i < sectors_count; i++) {
        while (!is_drive_ready()) {
            continue; // busy wait
        }
        drive_receive_sector();
    }
    assert(!current.running, "read must be finished");
}

void test()
{
    /* Test 1: Read MBR and check magic in the end of sector*/
    uint32_t lba = 0;
    uint32_t sectors = 1;
    uint8_t* buf = (uint8_t*)malloc(512 * sectors);
    drive_read_blocking(lba, sectors, buf);
    assert(buf[510] == 0x55, "mbr test");
    assert(buf[511] == 0xaa, "mbr test");
    free(buf);

    /* Test 2: Read few first sectors of the kernel code */
    lba = 1;
    sectors = 3;
    buf = (uint8_t*)malloc(512 * sectors);
    drive_read_blocking(lba, sectors, buf);
    for (int i = 0; i < 10; i++) {
        uint8_t disk = buf[i];
        uint8_t mem = ((uint8_t*)KERNEL_CODE_START)[i];
        assert(disk == mem, "kernel code test");
    }
    free(buf);
}

void reset_drive()
{
    klog(INFO, "resetting drive...");
    outb(DEVICE_CONTROL_REGISTER, 4); // reset
    outb(DEVICE_CONTROL_REGISTER, 0); // reset
    for (int i = 0; i < 15; i++) {
        inb(ALTERNATE_STATUS_REGISTER); // just wait 400ns
    }
    outb(DEVICE_CONTROL_REGISTER, 2); // disable IRQ
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert(!status.err && !status.df, "drive error");
    assert(status.rdy, "drive must be ready after reset");
    assert(!status.idx && !status.corr, "drive status is corrupted");

    klog(DEBUG, "drive status dump: ERR=%d IDX=%d CORR=%d DRQ=%d SRV=%d DF=%d RDY=%d BSY=%d", status.err, status.idx, status.corr, status.drq, status.srv, status.df, status.rdy, status.bsy);

    current = (Request) { 0 };
    test();
}

void init_drive()
{
    reset_drive();
}
