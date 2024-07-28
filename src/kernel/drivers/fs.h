#ifndef FS_H
#define FS_H

void init_filesystem();
void fs_read_blocking(char* filename, size_t offset, size_t count, void* buf);

#endif // FS_H
