#ifndef FS_H
#define FS_H

void init_filesystem();
int fs_read(char* filepath, size_t offset, size_t bytes, uint8_t* buf, bool yield);

#endif // FS_H
