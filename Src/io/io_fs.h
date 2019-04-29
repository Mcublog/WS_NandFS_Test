#ifndef IO_FS_H
#define IO_FS_H

#include "lfs.h"

int io_fs_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int io_fs_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int io_fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
int io_fs_flash_sync(const struct lfs_config *c);

#endif // IO_FS_H