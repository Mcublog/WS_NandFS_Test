#include "io_fs.h"

#include <stdint.h>
#include <assert.h>
#include "io_nand.h"

/*-----------------------------------------------------------
/brief: Read data from flash
/param: Pointer to lfs config
/param: Number block
/param: Page offset
/param: Pointer to buffer
/param: Number bytes to read
/return: 0 if all ok
-----------------------------------------------------------*/
int io_fs_flash_read(  const struct lfs_config *cfg, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size)
{
    assert(off  % cfg->read_size == 0);
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);   

    uint32_t addr = block * io_nand_get_block_size();
    io_nand_write_8b(addr, (uint8_t*) buffer, size, off % io_nand_get_page_size());

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
int io_fs_flash_prog(  const struct lfs_config *cfg, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size)
{

    assert(off  % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);  

    uint32_t addr = block * io_nand_get_block_size();
    io_nand_read_8b (addr, (uint8_t*) buffer, size, off % io_nand_get_page_size());
  
    return 0;
}

/*-----------------------------------------------------------
/brief: Erase block
/param: Pointer to lfs config
/param: Number block
/return: 0 if all ok
-----------------------------------------------------------*/
int io_fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    assert(block < cfg->block_count);  

    io_nand_erase(block * io_nand_get_block_size());

    return 0;
}

/*-----------------------------------------------------------
/brief: Sync data
/param: Pointer to lfs config
/return: 0 if all ok
-----------------------------------------------------------*/
int io_fs_flash_sync(const struct lfs_config *c)
{
    return 0;
}
