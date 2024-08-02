#include "../../include/string.h"
#include "../../include/utils.h"
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

uint32_t calc_filepath_depth(char* filepath)
{
    assert(filepath[0] == '/', "filepath must start from \"/\"");
    uint32_t depth = 0;
    while (*filepath != '\0') {
        if (*filepath == '/') {
            depth++;
        }
        filepath++;
    }
    if (depth > 1) {
        if (*(filepath - 1) == '/') {
            depth--;
        }
    }
    return depth;
}

char* copy_filename_level(char* filepath, uint32_t lookup_level, char* result)
{
    assert(filepath[0] == '/', "filepath must start from \"/\"");
    uint32_t current_level = -1;
    char* s = NULL;
    while (true) {
        if (*filepath == '/') {
            current_level++;
            if (current_level >= lookup_level) {
                s = filepath + 1;
                break;
            }
        }
        if (*filepath == '\0') {
            break;
        }
        filepath++;
    }
    assert(s != NULL, "path level %d was not found in filepath %s", lookup_level, filepath);
    uint32_t i = 0;
    while (s[i] != '\0' && s[i] != '/') {
        assert(i < 12, "filename can't be longer than 11: filepath=%s", s);
        if (s[i] >= 'a' && s[i] <= 'z') {
            result[i] = s[i] - 32; // upper case
        } else {
            result[i] = s[i];
        }
        i++;
    }
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
uint32_t BYTES_PER_CLUSTER;
uint32_t ROOT_DIR_FIRST_CLUSTER;
uint32_t CLUSTERS_LBA_BEGIN;
uint32_t FAT_PARTITION_LBA_BEGIN;
uint32_t FAT_TABLE_LBA_BEGIN;
uint32_t DIR_RECORD_SIZE;
uint32_t DIR_RECORDS_PER_CLUSTER; // each file/subdirectory in directory has size of 32 bytes
uint32_t FAT_TABLE_ENTRIES_PER_SECTOR;
const uint32_t NO_MORE_CLUSTERS = 0xFFFFFFFF;

#define MAX_QUEUE_SIZE 100

typedef struct {
    char short_name[12]; // max size of filename (11) + \0 byte
    uint32_t first_cluster;
    uint32_t file_size;
    bool is_dir;
} File;

File parse_file_entry(uint8_t* buf)
{
    uint8_t attrib = *((uint8_t*)(buf + 11));
    uint16_t first_cluster_hi = *((uint16_t*)(buf + 20));
    uint16_t first_cluster_lo = *((uint16_t*)(buf + 26));
    uint32_t file_size = *((uint32_t*)(buf + 28));
    bool read_only = (attrib >> 0) & 1; // Should not allow writing
    bool hidden = (attrib >> 1) & 1; // Should not show in dir listing
    bool system = (attrib >> 2) & 1; // File is operating system
    bool volume_id = (attrib >> 3) & 1; // Filename is Volume ID
    bool directory = (attrib >> 4) & 1; // Is a subdirectory
    bool archive = (attrib >> 5) & 1; // Has been changed since last backup

    bool attrib_has_zeros = !(((attrib >> 6) & 1) || (attrib >> 7) & 1);
    assert(attrib_has_zeros, "buf does not belong to directory");

    File file = { 0 };
    char* name = file.short_name;

    for (int i = 0; i < 8; i++) {
        if (buf[i] == 0x20) {
            break;
        }
        *name = buf[i];
        name++;
    }
    if (buf[8] != 0x20) {
        *name = '.';
        name++;
    }
    for (int i = 8; i < 11; i++) {
        if (buf[i] == 0x20) {
            break;
        }
        *name = buf[i];
        name++;
    }

    file.first_cluster = first_cluster_lo + (first_cluster_hi << 16);
    file.file_size = file_size;
    file.is_dir = directory;
    return file;
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

    FAT_PARTITION_LBA_BEGIN = partitions[0].lba_begin;
    FAT_TABLE_LBA_BEGIN = FAT_PARTITION_LBA_BEGIN + v.number_of_reserved_sectors;
    CLUSTERS_LBA_BEGIN = FAT_PARTITION_LBA_BEGIN + v.number_of_reserved_sectors + v.number_of_fats * v.sectors_per_fat;
    SECTORS_PER_CLUSTER = v.sectors_per_cluster;
    ROOT_DIR_FIRST_CLUSTER = v.root_directory_first_cluster;
    DIR_RECORD_SIZE = 32;
    DIR_RECORDS_PER_CLUSTER = 512 * SECTORS_PER_CLUSTER / DIR_RECORD_SIZE;
    FAT_TABLE_ENTRIES_PER_SECTOR = 128;
    BYTES_PER_CLUSTER = SECTORS_PER_CLUSTER * 512;
}

void fs_send_read_cluster_command(uint32_t cluster_no)
{
}

uint32_t cluster_to_lba(uint32_t cluster)
{
    assert(cluster >= 2, "cluster enumeration starts from 2");
    return CLUSTERS_LBA_BEGIN + (cluster - 2) * SECTORS_PER_CLUSTER;
}

/* return 0 if no more clusters available */
uint32_t fetch_next_cluster(uint32_t cluster, bool yield)
{
    uint8_t* buf = (uint8_t*)malloc(512);
    uint32_t lba = FAT_TABLE_LBA_BEGIN + cluster / FAT_TABLE_ENTRIES_PER_SECTOR;
    drive_read(lba, 1, buf, yield);
    free(buf);
    return ((uint32_t*)buf)[cluster % FAT_TABLE_ENTRIES_PER_SECTOR];
}

const int NO_SUCH_FILE = -1;
const int END_OF_FILE = 0;

bool find_file(uint32_t dir_first_cluster, char* filename, File* res, bool yield)
{
    uint32_t cluster = dir_first_cluster;
    uint8_t* buf = (uint8_t*)malloc(512 * SECTORS_PER_CLUSTER);
    // TODO: scan more clusters if file not found in first
    drive_read(
        cluster_to_lba(cluster),
        SECTORS_PER_CLUSTER,
        buf,
        yield);
    for (int i = 0; i < DIR_RECORDS_PER_CLUSTER; i++) {
        uint8_t* file_entry = buf + DIR_RECORD_SIZE * i;
        if (file_entry[0] == 0) {
            break;
        }
        if (file_entry[0] == 0xE5) {
            // empty
            continue;
        }
        File file = parse_file_entry(file_entry);
        klog(DEBUG, "dir contain file: \"%s\"", file.short_name);
        if (strcmp(file.short_name, filename) == 0) {
            *res = file;
            free(buf);
            return true;
        }
    }
    free(buf);
    return false;
}

/* from is including, to is excluding */
uint32_t read_file(File file, size_t from, size_t to, uint8_t* buf, bool yield)
{
    size_t cluster_idx = 0;
    uint32_t cluster = file.first_cluster;
    uint32_t bytes_read = 0;
    size_t total_clusters = (file.file_size + BYTES_PER_CLUSTER - 1) / (BYTES_PER_CLUSTER);
    uint8_t* tmp = (uint8_t*)malloc(BYTES_PER_CLUSTER);
    for (int cluster_idx = 0; cluster_idx < total_clusters; cluster_idx++) {
        if (from >= to) {
            break;
        }
        size_t l = cluster_idx * BYTES_PER_CLUSTER;
        size_t r = (cluster_idx + 1) * BYTES_PER_CLUSTER;

        if (from >= l) {
            drive_read(cluster_to_lba(cluster), SECTORS_PER_CLUSTER, tmp, yield);
            for (int i = from - l; i < MIN(r - l, to - l); i++) {
                *buf = tmp[i];
                buf++;
                bytes_read++;
            }
            from = r;
        }

        cluster = fetch_next_cluster(cluster, yield);
        assert(cluster != NO_MORE_CLUSTERS, "we iterate over calculated amount of clusters, no 0xFFFFFFF are expected as next cluster");
    }
    free(tmp);
    return bytes_read;
}

int fs_read(char* filepath, size_t offset, size_t bytes, uint8_t* buf, bool yield)
{
    klog(DEBUG, "reading file \"%s\"", filepath);
    uint32_t cluster = ROOT_DIR_FIRST_CLUSTER;
    uint32_t filepath_depth = calc_filepath_depth(filepath);
    File target;

    for (int i = 0; i < filepath_depth; i++) {
        char name[12] = { 0 };
        copy_filename_level(filepath, i, name);
        klog(DEBUG, "lookup of file \"%s\"", name);
        File file = { 0 };
        bool file_found = find_file(cluster, name, &file, yield);
        if (!file_found) {
            klog(DEBUG, "file was not found");
            return NO_SUCH_FILE;
        }
        cluster = file.first_cluster;
        klog(DEBUG, "file \"%s\" was found, first cluster=%d", name, cluster);
        if (i == filepath_depth - 1) {
            target = file;
        } else {
            if (!file.is_dir) {
                return NO_SUCH_FILE;
            }
        }
    }
    assert(cluster != NO_MORE_CLUSTERS, "cluster must be something else");
    klog(DEBUG, "read file %s size=%d", target.short_name, target.file_size);
    return read_file(target, offset, offset + MIN(bytes, target.file_size), buf, yield);
}

void fs_read_nonblocking(char* filename, size_t offset, size_t count, void* buf)
{
}

// void fs_run_timeslice()
// {
//     if (fs.queue == NULL) {
//         return; // nothing to do
//     }
//     Request* r = fs.queue;
//     assert(r->filepath[0] == '\\', "filename must start with \\");

//     // fs.buf contains previously read cluster

//     if (r->state == FILE_LOOKUP) {
//         if (r->lookup_level == 0) {
//             fs_send_read_cluster_command(fs.v.root_directory_first_cluster);
//             r->lookup_level++;
//             return;
//         }
//         char* lookup_file = copy_filename_level(r->filepath, r->lookup_level);

//         File files[16];
//         int file_index = -1;
//         for (int i = 0; i < 16; i++) {
//             if (strcmp(files[i].short_name, lookup_file)) {
//                 file_index = i;
//                 break;
//             }
//         }
//         uint32_t next_cluster_no;
//         if (file_index == -1) {
//             next_cluster_no = 10; // find next cluster in FAT table
//         } else {
//             //
//         }
//     }
// }

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
