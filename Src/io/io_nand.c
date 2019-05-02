#include "io_nand.h"

#include "stm32f4xx_hal.h"
#include "init_main.h"

//-----------------------Local variables and fucntion-------------------------
uint32_t _nand_read_8b (NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset);
uint32_t _nand_write_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset);
uint32_t            _flash_adr_to_uint32(NAND_AddressTypeDef *adr);
NAND_AddressTypeDef _uint32_to_flash_adr(uint32_t adr);

extern NAND_HandleTypeDef hnand1;
static io_nand_cfg_t _nand_cfg = {0};
//----------------------------------------------------------------------------

/*-----------------------------------------------------------
/brief: Init NAND
/param:
/return: 0 -- if all is good
-----------------------------------------------------------*/
uint32_t io_nand_init(void)
{
    MX_FSMC_Init();
    io_nand_init_cfg();
    return 0;
}

/*-----------------------------------------------------------
/brief: Init NAND Config from hnand -> Config
/param:
/return: 0 -- if all is good
-----------------------------------------------------------*/
uint32_t io_nand_init_cfg()
{
    //Get config data from hnand1
    _nand_cfg.page_size     = hnand1.Config.PageSize; // Page size (2048 for K9GAG08U0E )
    _nand_cfg.block_number  = hnand1.Config.BlockNbr; // Total Number of block in plane (1024 K9GAG08U0E )
    _nand_cfg.block_size    = hnand1.Config.BlockSize; // Block size (In page) (64 K9GAG08U0E )
    _nand_cfg.plane_number  = hnand1.Config.PlaneNbr;  // Number of plane (1 K9GAG08U0E )
    _nand_cfg.plane_size    = hnand1.Config.PlaneSize * _nand_cfg.block_number; // Plane size (In Page)
    return 0;
}

/*-----------------------------------------------------------
/brief: Set NAND config
/param: Size page
/param: Block number
/param: Block size
/param: Plane number
/return: 0 -- if all is good, 1 -- if parameters do not match hnand1->Config
-----------------------------------------------------------*/
uint32_t io_nand_set_cfg(uint32_t p_size, uint32_t b_num, uint32_t b_size, uint32_t pl_num)
{
    if (p_size > hnand1.Config.PageSize)     return 1;
    if (b_num  > hnand1.Config.BlockNbr)     return 1;
    if (b_size > hnand1.Config.BlockSize)    return 1;
    if (pl_num > hnand1.Config.PlaneNbr)     return 1;

    _nand_cfg.page_size     = p_size;
    _nand_cfg.block_number  = b_num;
    _nand_cfg.block_size    = b_size;
    _nand_cfg.plane_number  = pl_num;
    _nand_cfg.plane_size    = pl_num * _nand_cfg.block_number; // Plane size (In Page)
    
    return 0;
}

/*-----------------------------------------------------------
/brief: NAND Flash get config
/param: Pointer to config
/return:
-----------------------------------------------------------*/
void io_nand_get_cfg(io_nand_cfg_t *cfg)
{
    *cfg = _nand_cfg;
}

/*-----------------------------------------------------------
/brief: NAND Flash get block size
/param:
/return: Block size (In page)
-----------------------------------------------------------*/
uint32_t io_nand_get_block_size(void)
{
    return _nand_cfg.block_size;
}

/*-----------------------------------------------------------
/brief: NAND Flash get block number
/param:
/return: Block size (In page)
-----------------------------------------------------------*/
uint32_t io_nand_get_block_number(void)
{
    return _nand_cfg.block_number;
}

/*-----------------------------------------------------------
/brief: NAND Flash get plane number
/param:
/return: Number of plane
-----------------------------------------------------------*/
uint32_t io_nand_get_plane_number(void)
{
    return _nand_cfg.plane_number;
}

/*-----------------------------------------------------------
/brief: NAND Flash get plane size (In Blocks)
/param:
/return: Plane size (In Blocks)
-----------------------------------------------------------*/
uint32_t io_nand_get_plane_size(void)
{
    return _nand_cfg.plane_size;
}

/*-----------------------------------------------------------
/brief: NAND Flash get page size
/param:
/return: Page size (In Bytes)
-----------------------------------------------------------*/
uint32_t io_nand_get_page_size(void)
{
    return _nand_cfg.page_size;
}

/*-----------------------------------------------------------
/brief: NAND Flash read data
/param: Address
/param: Pointer to read buffer
/param: Number bytes to read
/param: Offset in page
/return: 0 -- if all is good
-----------------------------------------------------------*/
uint32_t io_nand_read_8b (uint32_t addr, uint8_t *buffer, uint32_t size, uint32_t offst)
{
    //Convert addr -> to HAL_NAND_addr
    NAND_AddressTypeDef a = _uint32_to_flash_adr(addr);
    _nand_read_8b (&hnand1, &a, buffer, size, 0);
    return 0;
}

/*-----------------------------------------------------------
/brief: NAND Flash write data
/param: Address
/param: Pointer to write buffer
/param: Number bytes to write
/param: Offset in page
/return: 0 -- if all is good
-----------------------------------------------------------*/
uint32_t io_nand_write_8b(uint32_t addr, uint8_t *buffer, uint32_t size, uint32_t offst)
{
    //Convert addr -> to HAL_NAND_addr
    NAND_AddressTypeDef a = _uint32_to_flash_adr(addr);
    _nand_write_8b(&hnand1, &a, buffer, size, offst);
    return 0;    
}

/*-----------------------------------------------------------
/brief: NAND Flash erase block
/param: Block address
/return:
-----------------------------------------------------------*/
void io_nand_erase(uint32_t addr)
{
    //Convert addr -> to HAL_NAND_addr
    NAND_AddressTypeDef a = _uint32_to_flash_adr(addr);
    HAL_NAND_Erase_Block(&hnand1, &a);
}

/*-----------------------------------------------------------
/brief: NAND Flash read data
/param: Pointer to NAND handle
/param: Pointer to NAND address
/param: Pointer to buffer
/param: Number bytes to read
/param: Offset in page
/return: 0 -- if all is good
-----------------------------------------------------------*/
uint32_t _nand_read_8b (NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset)
{
    __IO uint32_t index  = 0U;
    uint32_t tickstart = 0U;
    uint32_t deviceaddress = 0U, nandaddress = 0U;
  
    /* Process Locked */
    __HAL_LOCK(hnand); 
  
    /* Check the NAND controller state */
    if(hnand->State == HAL_NAND_STATE_BUSY)
    {
        return HAL_BUSY;
    }
  
    /* Identify the device address */
    if(hnand->Init.NandBank == FMC_NAND_BANK2)
    {
        deviceaddress = NAND_DEVICE1;
    }
    else
    {
        deviceaddress = NAND_DEVICE2;
    }

    /* Update the NAND controller state */
    hnand->State = HAL_NAND_STATE_BUSY;
  
    /* NAND raw address calculation */
    nandaddress = ARRAY_ADDRESS(pAddress, hnand);
  
    /* Send read page command sequence */
    *(__IO uint8_t *)((uint32_t)(deviceaddress | CMD_AREA)) = NAND_CMD_AREA_A;
   
    /* Cards with page size <= 512 bytes */
    if((hnand->Config.PageSize) <= 512U)
    {
        if (((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) <= 65535U)
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
        }
        else /* ((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) > 65535 */
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_3RD_CYCLE(nandaddress);
        }
    }
    else /* (hnand->Config.PageSize) > 512 */
    {
        if (((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) <= 65535U)
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_2ND_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
        }
        else /* ((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) > 65535 */
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_2ND_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_3RD_CYCLE(nandaddress);
        }
    }
  
    *(__IO uint8_t *)((uint32_t)(deviceaddress | CMD_AREA))  = NAND_CMD_AREA_TRUE1;
      
    /* Check if an extra command is needed for reading pages  */
    if(hnand->Config.ExtraCommandEnable == ENABLE)
    {
        /* Get tick */
        tickstart = HAL_GetTick();
      
        /* Read status until NAND is ready */
        while(HAL_NAND_Read_Status(hnand) != NAND_READY)
        {
            if((HAL_GetTick() - tickstart ) > NAND_WRITE_TIMEOUT)
            {
                return HAL_TIMEOUT; 
            }
        }
      
        /* Go back to read mode */
        *(__IO uint8_t *)((uint32_t)(deviceaddress | CMD_AREA)) = ((uint8_t)0x00);
        __DSB();
    }
    
    /* Get Data into Buffer */    
    for(; index < size; index++)
    {
        *(uint8_t *)pBuffer++ = *(uint8_t *)deviceaddress;
    }
  
    /* Update the NAND controller state */
    hnand->State = HAL_NAND_STATE_READY;
  
    /* Process unlocked */
    __HAL_UNLOCK(hnand);

    return HAL_OK;
}

/*-----------------------------------------------------------
/brief: NAND Flash write data
/param: Pointer to NAND handle
/param: Pointer to NAND address
/param: Pointer to buffer
/param: Number bytes to write
/param: Offset in page
/return: 0 -- if all is good
-----------------------------------------------------------*/
uint32_t _nand_write_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset)
{
    __IO uint32_t index = 0U;
    uint32_t tickstart = 0U;
    uint32_t deviceaddress = 0U, nandaddress = 0U;
  
    /* Process Locked */
    __HAL_LOCK(hnand);  

    /* Check the NAND controller state */
    if(hnand->State == HAL_NAND_STATE_BUSY)
    {
        return HAL_BUSY;
    }
  
    /* Identify the device address */
    if(hnand->Init.NandBank == FMC_NAND_BANK2)
    {
        deviceaddress = NAND_DEVICE1;
    }
    else
    {
        deviceaddress = NAND_DEVICE2;
    }
  
    /* Update the NAND controller state */ 
    hnand->State = HAL_NAND_STATE_BUSY;
  
    /* NAND raw address calculation */
    nandaddress = ARRAY_ADDRESS(pAddress, hnand);
       
    /* Send write page command sequence */
    *(__IO uint8_t *)((uint32_t)(deviceaddress | CMD_AREA)) = NAND_CMD_AREA_A;
    *(__IO uint8_t *)((uint32_t)(deviceaddress | CMD_AREA)) = NAND_CMD_WRITE0;

    /* Cards with page size <= 512 bytes */
    if((hnand->Config.PageSize) <= 512U)
    {
        if (((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) <= 65535U)
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
        }
        else /* ((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) > 65535 */
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_3RD_CYCLE(nandaddress);
        }
    }
    else /* (hnand->Config.PageSize) > 512 */
    {
        if (((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) <= 65535U)
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_2ND_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
        }
        else /* ((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) > 65535 */
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_2ND_CYCLE(offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
            __DSB();
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_3RD_CYCLE(nandaddress);
            __DSB();
        }
    }
  

    /* Write data to memory */
    for(; index < size; index++)
    {
        *(__IO uint8_t *)deviceaddress = *(uint8_t *)pBuffer++;
    }
   
    *(__IO uint8_t *)((uint32_t)(deviceaddress | CMD_AREA)) = NAND_CMD_WRITE_TRUE1;
    
    /* Read status until NAND is ready */
    while(HAL_NAND_Read_Status(hnand) != NAND_READY)
    {
        /* Get tick */
        tickstart = HAL_GetTick();
      
        if((HAL_GetTick() - tickstart ) > NAND_WRITE_TIMEOUT)
        {
            return HAL_TIMEOUT; 
        }
    }
  
    /* Update the NAND controller state */ 
    hnand->State = HAL_NAND_STATE_READY;
  
    /* Process unlocked */
    __HAL_UNLOCK(hnand);
  
    return HAL_OK;
}

/*-----------------------------------------------------------
/brief:  Convert adr NAND_AddressTypeDef -> uint32_t
/param:  Addr in uint32_t format
/return: NAND addr in NAND_AddressTypeDef format
-----------------------------------------------------------*/
uint32_t _flash_adr_to_uint32(NAND_AddressTypeDef *adr)
{
	uint32_t a = (adr -> Plane * _nand_cfg.plane_size) + (adr -> Block * _nand_cfg.block_size) + adr -> Page;
	return a;
}

/*-----------------------------------------------------------
/brief:  Convert adr uint32_t -> NAND_AddressTypeDef
/param:  Addr in uint32_t format
/return: NAND addr in NAND_AddressTypeDef format
-----------------------------------------------------------*/
NAND_AddressTypeDef _uint32_to_flash_adr(uint32_t adr)
{
	NAND_AddressTypeDef a;
	a.Plane = adr / _nand_cfg.plane_size;
	a.Block = (adr - a.Plane * _nand_cfg.plane_size) / _nand_cfg.block_size;
	a.Page  = adr - (a.Plane * _nand_cfg.plane_size) - (a.Block * _nand_cfg.block_size);

	return a;
}
