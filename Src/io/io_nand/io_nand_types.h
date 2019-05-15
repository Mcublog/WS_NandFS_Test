#ifndef IO_NAND_TYPES
#define IO_NAND_TYPES

#include <stdint.h>

typedef struct
{
    uint32_t page_size;     // Page size
    uint32_t block_number;  // TOTAL Number of block in plane 
    uint32_t block_size;    // Block size (In Page)
    uint32_t plane_number;  // Number of plane
    uint32_t plane_size;    // Plane size (In Page)
}io_nand_cfg_t;

typedef struct
{
    io_nand_cfg_t cfg;
    void          *pnandh;// Pointer to NAND Handler
}io_nand_t;

#ifndef HAL_NAND_MODULE_ENABLED
/** 
  * @brief  NAND Memory address Structure definition
  */
typedef struct 
{
  uint16_t Page;   /*!< NAND memory Page address    */

  uint16_t Plane;   /*!< NAND memory Plane address  */

  uint16_t Block;  /*!< NAND memory Block address   */

}NAND_AddressTypeDef;
#endif


#endif // IO_NAND_TYPES
