#include "io_fs.h"

#include <stdint.h>
#include <assert.h>
#include "io_nand.h"

//-----------------------Local variables and function-------------------------
int _fs_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int _fs_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int _fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
int _fs_flash_sync(const struct lfs_config *c);

static lfs_t    _lfs = {0};
static struct lfs_config _lfs_config = {0};
static uint8_t  _rd[2048] = {0};
static uint8_t  _wr[2048] = {0};
//----------------------------------------------------------------------------

/*-----------------------------------------------------------
/brief: Read data from flash
/param: Pointer to lfs config
/param: Number block
/param: Page offset
/param: Pointer to buffer
/param: Number bytes to read
/return: 0 if all ok
-----------------------------------------------------------*/
int _fs_flash_read(  const struct lfs_config *cfg, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size)
{
    assert(off  % cfg->read_size == 0);
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);   

    uint32_t addr = block * io_nand_get_block_size() + off / io_nand_get_page_size();
    io_nand_read(addr, (uint8_t*) buffer, size, 0);

    return 0;
}

/*-----------------------------------------------------------
/brief: Program data to flash
/param: Pointer to lfs config
/param: Number block
/param: Page offset
/param: Pointer to buffer
/param: Number bytes to write
/return: 0 if all ok
-----------------------------------------------------------*/
int _fs_flash_prog(  const struct lfs_config *cfg, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size)
{

    assert(off  % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);  

    uint32_t addr = block * io_nand_get_block_size() + off / io_nand_get_page_size();
    io_nand_write(addr, (uint8_t*) buffer, size, 0);
  
    return 0;
}

/*-----------------------------------------------------------
/brief: Erase block
/param: Pointer to lfs config
/param: Number block
/return: 0 if all ok
-----------------------------------------------------------*/
int _fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    assert(block < cfg->block_count);  

    uint32_t addr = block * io_nand_get_block_size();
    io_nand_block_erase(addr);

    return 0;
}

/*-----------------------------------------------------------
/brief: Sync data
/param: Pointer to lfs config
/return: 0 if all ok
-----------------------------------------------------------*/
int _fs_flash_sync(const struct lfs_config *c)
{
    return 0;
}

/*-----------------------------------------------------------
/brief: Init FS
/param:
/return: 0 if all ok
-----------------------------------------------------------*/
int io_fs_init(void)
{
    uint32_t page_size = io_nand_get_page_size();

    _lfs_config.read_size   = page_size;
    _lfs_config.prog_size   = page_size;
    
    _lfs_config.block_size  = io_nand_get_block_size() * page_size;
    _lfs_config.block_count = io_nand_get_block_number();
    
    _lfs_config.block_cycles = 100;
    _lfs_config.lookahead_size = page_size;
    _lfs_config.cache_size     = page_size;
    
    _lfs_config.read_buffer = _rd;
    _lfs_config.prog_buffer = _wr;

    _lfs_config.read   = _fs_flash_read;
    _lfs_config.prog   = _fs_flash_prog;
    _lfs_config.erase  = _fs_flash_erase;
    _lfs_config.sync   = _fs_flash_sync;

    return 0;
}

/*-----------------------------------------------------------
/brief: Format a block device with the FS
/param:
/return: A negative error code on failure
-----------------------------------------------------------*/
int io_fs_format(void)
{
    return lfs_format(&_lfs, &_lfs_config);
}

/*-----------------------------------------------------------
/brief: Mount a FS
/param:
/return: A negative error code on failure
-----------------------------------------------------------*/
int io_fs_mount(void)
{
    return lfs_mount(&_lfs, &_lfs_config);
}

/*-----------------------------------------------------------
/brief: Unmount a FS
/param:
/return: A negative error code on failure
-----------------------------------------------------------*/
int io_fs_unmount()
{
    return lfs_unmount(&_lfs);
}

/*-----------------------------------------------------------
/brief: Open a file
/param: Pointer to the io_fs_file
/param: Pointer to the sting with path to file
/param: Flags, that are bitwise-ored together.
/return: A negative error code on failure
-----------------------------------------------------------*/
int io_fs_file_open(io_fs_file *file, const char *path, int32_t flags)
{
    return lfs_file_open(&_lfs, &file->f, path, flags);
}

/*-----------------------------------------------------------
/brief: Close a file
/param: Pointer to the io_fs_file
/return: A negative error code on failure
-----------------------------------------------------------*/
int io_fs_file_close(io_fs_file *file)
{
    return lfs_file_close(&_lfs, &file->f);
}

/*-----------------------------------------------------------
/brief: Read data from file
/param: Pointer to the io_fs_file
/param: Pointer to the buffer
/param: Size of data
/return: the number of bytes read, or a negative error code on failure
-----------------------------------------------------------*/
int io_fs_file_read(io_fs_file *file, void *buffer, uint32_t size)
{
    return lfs_file_read(&_lfs, &file->f, buffer, size);
}

/*-----------------------------------------------------------
/brief: Write data to file
/param: Pointer to the io_fs_file
/param: Pointer to the buffer
/param: Size of data
/return: the number of bytes written, or a negative error code on failure
-----------------------------------------------------------*/
int io_fs_file_write(io_fs_file *file, void *buffer, uint32_t size)
{
    return lfs_file_write(&_lfs, &file->f, buffer, size);
}

/*-----------------------------------------------------------
/brief: Change the position of the file to the beginning of the file
/param: Pointer to the io_fs_file
/return: the number of bytes written, or a negative error code on failure
-----------------------------------------------------------*/
int io_fs_file_rewind(io_fs_file *file)
{
    return lfs_file_rewind(&_lfs, &file->f);
}
