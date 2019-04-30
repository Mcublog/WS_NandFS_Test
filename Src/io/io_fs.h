#ifndef IO_FS_H
#define IO_FS_H

#include "lfs.h"

typedef struct 
{
    lfs_file_t f;
}io_fs_file;

// File open flags
enum io_fs_open_flags {
    // open flags
    IO_FS_O_RDONLY = 1,         // Open a file as read only
    IO_FS_O_WRONLY = 2,         // Open a file as write only
    IO_FS_O_RDWR   = 3,         // Open a file as read and write
    IO_FS_O_CREAT  = 0x0100,    // Create a file if it does not exist
    IO_FS_O_EXCL   = 0x0200,    // Fail if a file already exists
    IO_FS_O_TRUNC  = 0x0400,    // Truncate the existing file to zero size
    IO_FS_O_APPEND = 0x0800,    // Move to end of file on every write

    // internally used flags
    IO_FS_F_DIRTY   = 0x010000, // File does not match storage
    IO_FS_F_WRITING = 0x020000, // File has been written since last flush
    IO_FS_F_READING = 0x040000, // File has been read since last flush
    IO_FS_F_ERRED   = 0x080000, // An error occured during write
    IO_FS_F_INLINE  = 0x100000, // Currently inlined in directory entry
};

// File seek flags
enum io_fs_whence_flags {
    IO_FS_SEEK_SET = 0,   // Seek relative to an absolute position
    IO_FS_SEEK_CUR = 1,   // Seek relative to the current file position
    IO_FS_SEEK_END = 2,   // Seek relative to the end of the file
};

int io_fs_init(void);

int io_fs_format(void);
int io_fs_mount(void);
int io_fs_unmount(void);

int io_fs_file_open(io_fs_file *file, const char *path, int32_t flags);

int io_fs_file_close(io_fs_file *file);
int io_fs_file_read(io_fs_file *file, void *buffer,  uint32_t size);
int io_fs_file_write(io_fs_file *file, void *buffer, uint32_t size);

int io_fs_file_rewind(io_fs_file *file);


#endif // IO_FS_H
