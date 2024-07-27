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

typedef struct Request Request;

typedef struct {

    uint32_t fetched_cluster;
    /*
        lookup of files is split to levels
        first level in   "/" root directory
        second level in  "/home/" first subdirectory
        last level reads "/home/hello.txt" file itself
    */
    uint32_t current_dir_level;
    bool fetched_file;
} State;

struct Request {
    char* filepath;
    void* buf;
    size_t size;
    uint32_t offset;
    RequestType type;
    uint32_t request_id;
    State state;
};

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

uint32_t SECTORS_PER_CLUSTER;
uint32_t ROOT_DIR_FIRST_CLUSTER;
uint32_t CLUSTERS_LBA_BEGIN;
uint32_t FAT_LBA_BEGIN;

#define MAX_QUEUE_SIZE 100

typedef struct {
    char short_name[11];
    uint32_t first_cluster;
    uint32_t file_size;
    bool is_dir;
} File;

File parse_file_entry(uint8_t* buf)
{
}

void parse_files(uint8_t* from, File* to, size_t files_count)
{
    uint32_t* files = from;
}

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

    assert(v.number_of_fats == 2, "count of fat tables must be 2");

    FAT_LBA_BEGIN = partitions[0].lba_begin;
    CLUSTERS_LBA_BEGIN = FAT_LBA_BEGIN + v.number_of_reserved_sectors + v.number_of_fats * v.sectors_per_fat;
    SECTORS_PER_CLUSTER = v.sectors_per_cluster;
    ROOT_DIR_FIRST_CLUSTER = v.root_directory_first_cluster;
}

void fs_send_read_cluster_command(uint32_t cluster_no)
{
}

uint32_t cluster_to_lba(uint32_t cluster)
{
    return CLUSTERS_LBA_BEGIN + cluster * SECTORS_PER_CLUSTER;
}

void fs_read_blocking(char* filename, size_t offset, size_t count, void* buf)
{
    void* buf = malloc(512 * SECTORS_PER_CLUSTER);
    drive_send_read_command(
        cluster_to_lba(ROOT_DIR_FIRST_CLUSTER),
        SECTORS_PER_CLUSTER,
        buf);
    while (!is_drive_ready()) {
        continue;
    }
    File* files = malloc(sizeof(File) * 16 * SECTORS_PER_CLUSTER);

}

void fs_read_nonblocking(char* filename, size_t offset, size_t count, void* buf)
{
}

char* get_level_from_filepath(char* filepath, uint32_t lookup_level)
{
    uint32_t current = 0;
    char* res = filepath;
    while (*filepath != '\0') {
        if (*filepath == '\\') {
            current++;
            if (current > lookup_level) {
                break;
            } else if (current == lookup_level) {
                res = filepath + 1;
            }
        }
        filepath++;
    }
    return res;
}

bool strcmp(char* a, char* b)
{
    return true;
}

void fs_run_timeslice()
{
    if (fs.queue == NULL) {
        return; // nothing to do
    }
    Request* r = fs.queue;
    assert(r->filepath[0] == '\\', "filename must start with \\");

    // fs.buf contains previously read cluster

    if (r->state == FILE_LOOKUP) {
        if (r->lookup_level == 0) {
            fs_send_read_cluster_command(fs.v.root_directory_first_cluster);
            r->lookup_level++;
            return;
        }
        char* lookup_file = get_level_from_filepath(r->filepath, r->lookup_level);

        File files[16];
        int file_index = -1;
        for (int i = 0; i < 16; i++) {
            if (strcmp(files[i].short_name, lookup_file)) {
                file_index = i;
                break;
            }
        }
        uint32_t next_cluster_no;
        if (file_index == -1) {
            next_cluster_no = 10; // find next cluster in FAT table
        } else {
            //
        }
    }
}

// void enqueue_drive_request(void* buf, size_t size, uint32_t lba, RequestType type)
// {

//     Request* cur = queue;
//     Request* last = NULL;
//     int queue_size = 0;
//     while (cur != NULL) {
//         last = cur;
//         cur = cur->next;
//         queue_size += 1;
//     }
//     if (queue_size >= MAX_QUEUE_SIZE) {
//         klog(FATAL, "ATA driver queue size is too big, can't enqueue request");
//         panic();
//     }
//     Request* request = (Request*)malloc(sizeof(Request));
//     request->buf = buf;
//     request->size = size;
//     request->type = type;
//     if (last != NULL) {
//         last->next = request;
//     } else {
//         queue = request;
//     }
// }

// void enqueue_drive_read(void* buf, size_t size, size_t lba)
// {
//     return enqueue_drive_request(buf, size, lba, READ);
// }

// void enqueue_drive_write(void* buf, size_t size, size_t lba)
// {
//     return enqueue_drive_request(buf, size, lba, WRITE);
// }
