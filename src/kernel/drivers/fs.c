#include "../libk/assert.h"
#include "../libk/log.h"
#include "../libk/memory.h"
#include "ata.h"
#include "low_level.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
    READ,
    WRITE,
} RequestType;

typedef struct DriverRequest DriverRequest;

struct DriverRequest {
    void* buf;
    size_t size;
    RequestType type;

    DriverRequest* next;
};

DriverRequest* queue;

typedef struct {
    uint8_t type;
    uint32_t lba_begin;
    uint32_t sectors;
} Partition;

Partition parse_partition(uint8_t* buf)
{
    uint8_t* type = (uint8_t*)(buf + 4);
    uint32_t* lba_begin = (uint32_t*)(buf + 8);
    uint32_t* sectors = (uint32_t*)(buf + 12);
    return (Partition) {
        .type = *type,
        .lba_begin = *lba_begin,
        .sectors = *sectors,
    };
}

typedef struct {
    uint16_t bytes_per_sector; // BPB_BytsPerSec | Offset: 0x0B | Size: 16 Bits | Always 512 Bytes
    uint8_t sectors_per_cluster; // BPB_SecPerClus | Offset: 0x0D | Size: 8 Bits | Values: 1,2,4,8,16,32,64,128
    uint16_t number_of_reserved_sectors; // BPB_RsvdSecCnt | Offset: 0x0E | Size: 16 Bits | Usually 0x20
    uint8_t number_of_fats; // BPB_NumFATs | Offset: 0x10 | Size: 8 Bits | Always 2
    uint32_t sectors_per_fat; // BPB_FATSz32 | Offset: 0x24 | Size: 32 Bits | Depends on disk size
    uint32_t root_directory_first_cluster; // BPB_RootClus | Offset: 0x2C | Size: 32 Bits | Usually 0x00000002
    uint16_t signature; // Signature | Offset: 0x1FE | Size: 16 Bits | Always 0xAA55
} VolumeID;

VolumeID parse_volume_id(uint8_t* buf)
{
    uint16_t* bytes_per_sector = (uint16_t*)(buf + 0x0B);
    uint8_t* sectors_per_cluster = (uint8_t*)(buf + 0x0D);
    uint16_t* number_of_reserved_sectors = (uint16_t*)(buf + 0x0E);
    uint8_t* number_of_fats = (uint8_t*)(buf + 0x10);
    uint32_t* sectors_per_fat = (uint32_t*)(buf + 0x24);
    uint32_t* root_directory_first_cluster = (uint32_t*)(buf + 0x2C);
    uint16_t* signature = (uint16_t*)(buf + 0x1FE);
    return (VolumeID) {
        .bytes_per_sector = *bytes_per_sector,
        .sectors_per_cluster = *sectors_per_cluster,
        .number_of_reserved_sectors = *number_of_reserved_sectors,
        .number_of_fats = *number_of_fats,
        .sectors_per_fat = *sectors_per_fat,
        .root_directory_first_cluster = *root_directory_first_cluster,
        .signature = *signature,
    };
}

#define MAX_QUEUE_SIZE 100

void init_filesystem()
{
    uint8_t* mbr_buf = (uint8_t*)malloc(512);
    drive_read_blocking(0, 1, mbr_buf);
    assert(mbr_buf[510] == 0x55, "mbr is corrupted");
    assert(mbr_buf[511] == 0xaa, "mbr is corrupted");
    Partition partitions[4] = { 0 };
    for (int i = 0; i < 4; i++) {
        Partition p = parse_partition(mbr_buf + 446 + i * 16);
        partitions[i] = p;
        klog(DEBUG, "parition #%u found type=%u lba_begin=%u sectors=%u", i + 1, p.type, p.lba_begin, p.sectors);
    }
    assert(partitions[0].type == 0x0C, "partition must have FAT32 type");

    // Read first sector of FAT partition containing Volume ID
    uint8_t* volume_id_buf = (uint8_t*)malloc(512);
    drive_read_blocking(partitions[0].lba_begin, 1, volume_id_buf);
    VolumeID v = parse_volume_id(volume_id_buf);

    assert(v.signature == 0xaa55, "fat32 signature is wrong");
    klog(INFO, "found FAT filesystem at partition #1");
    klog(DEBUG, "fat parameters dump: bytes_per_sector=%d sectors_per_cluster=%d number_of_reserved_sectors=%d number_of_fats=%d sectors_per_fat=%d root_directory_first_cluster=%d", v.bytes_per_sector, v.sectors_per_cluster, v.number_of_reserved_sectors, v.number_of_fats, v.sectors_per_fat, v.root_directory_first_cluster);
}

void enqueue_drive_request(void* buf, size_t size, uint32_t lba, RequestType type)
{

    DriverRequest* cur = queue;
    DriverRequest* last = NULL;
    int queue_size = 0;
    while (cur != NULL) {
        last = cur;
        cur = cur->next;
        queue_size += 1;
    }
    if (queue_size >= MAX_QUEUE_SIZE) {
        klog(FATAL, "ATA driver queue size is too big, can't enqueue request");
        panic();
    }
    DriverRequest* request = (DriverRequest*)malloc(sizeof(DriverRequest));
    request->buf = buf;
    request->size = size;
    request->type = type;
    if (last != NULL) {
        last->next = request;
    } else {
        queue = request;
    }
}

void enqueue_drive_read(void* buf, size_t size, size_t lba)
{
    return enqueue_drive_request(buf, size, lba, READ);
}

void enqueue_drive_write(void* buf, size_t size, size_t lba)
{
    return enqueue_drive_request(buf, size, lba, WRITE);
}
