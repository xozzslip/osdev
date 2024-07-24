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

void assert_drive_health(Status status)
{
    bool ok = true;
    if (status.err || status.df) {
        klog(FATAL, "drive check failed: ERR or DF flag is set");
        ok = false;
    }
    if (!status.rdy) {
        klog(FATAL, "drive check failed: RDY flag is not set");
        ok = false;
    }
    if (status.idx || status.corr) {
        klog(FATAL, "drive check failed: IDX or CORR flags are set");
        ok = false;
    }
    if (!ok) {
        klog(FATAL, "drive status dump: ERR=%d IDX=%d CORR=%d DRQ=%d SRV=%d DF=%d RDY=%d BSY=%d", status.err, status.idx, status.corr, status.drq, status.srv, status.df, status.rdy, status.bsy);
        panic();
    }
}

typedef enum {
    READ,
    WRITE,

} RequestType;

typedef struct {
    RequestType type;
    uint32_t lba;
    uint32_t sectors_count;
    void* buf;
    bool running;
} Request;

Request request_in_progress;

void reset_drive()
{
    klog(DEBUG, "resetting drive...");
    outb(DEVICE_CONTROL_REGISTER, 4); // reset
    outb(DEVICE_CONTROL_REGISTER, 0); // reset
    for (int i = 0; i < 15; i++) {
        inb(ALTERNATE_STATUS_REGISTER); // just wait 400ns
    }
    outb(DEVICE_CONTROL_REGISTER, 2); // disable IRQ
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert_drive_health(status);
    request_in_progress.running = 0;
}

void init_drive()
{
    reset_drive();
}

void drive_send_read_command(uint32_t lba, uint32_t sectors_count, void* buf)
{
    if (lba >= (1 << 28)) {
        klog(FATAL, "failed to send read command to drive: lba address can't must be less than 28 bits number");
        panic();
    }
    if (sectors_count > (1 << 8)) {
        klog(FATAL, "failed to send read command to drive: sectors count can't be greater than 256");
        panic();
    }
    if (request_in_progress.running) {
        klog(FATAL, "failed to send read command to drive: drive is already busy");
        panic();
    }
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert_drive_health(status);
    if (status.bsy) {
        klog(FATAL, "failed to send read command to drive: device expected to be not busy");
        panic();
    }

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

    request_in_progress.lba = lba;
    request_in_progress.sectors_count = sectors_count;
    request_in_progress.type = READ;
    request_in_progress.running = true;
    request_in_progress.buf = buf;
}

bool drive_data_bus_is_ready() {
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert_drive_health(status);
    return !status.bsy && status.drq;
}

void drive_receive_data() {
    Status status = parse_status(inb(ALTERNATE_STATUS_REGISTER));
    assert_drive_health(status);
    if (!drive_data_bus_is_ready()) {
        klog(FATAL, "failed to receive data from drive: data is not ready");
    }
}
