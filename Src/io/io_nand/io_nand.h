#ifndef IO_NAND_H
#define IO_NAND_H

#include <stdint.h>

#include "io_nand_types.h"

uint32_t io_nand_init(void);

uint32_t io_nand_set_cfg(uint32_t p_size, uint32_t b_num, uint32_t b_size, uint32_t pl_num);
void     io_nand_get_cfg(io_nand_cfg_t *cfg);

uint32_t io_nand_get_page_size(void);
uint32_t io_nand_get_block_number(void);
uint32_t io_nand_get_block_size(void);
uint32_t io_nand_get_plane_number(void);
uint32_t io_nand_get_plane_size(void);

uint32_t io_nand_read (uint32_t addr, uint8_t *buffer, uint32_t size, uint32_t offst);
uint32_t io_nand_write(uint32_t addr, uint8_t *buffer, uint32_t size, uint32_t offst);
void     io_nand_block_erase(uint32_t addr);

#endif // IO_NAND_H
