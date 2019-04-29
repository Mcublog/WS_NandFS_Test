#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

#include "init_main.h"
#include "test.h"
//-----------------------Types and definition---------------------------------
//Redefine in stm32f4xx_hal_nand.h for Waveshare board
#define CMD_AREA                   ((uint32_t)(1U<<17U))  /* A16 = CLE high */
#define ADDR_AREA                  ((uint32_t)(1U<<16U))  /* A17 = ALE high */

#define PAGE_NMBR	(64)
#define BLOCK_NMBR	(1024)
#define PLANE_NMBR  (1)

#define LAST_PAGE	(PAGE_NMBR - 1)
#define LAST_BLOCK	(BLOCK_NMBR - 1)
#define LAST_PLANE	(PLANE_NMBR - 1)

#define PAGE_SIZE  (2048)
#define PLANE_SIZE (BLOCK_NMBR * PAGE_NMBR)//Plane size in page
#define BLOCK_SIZE (PAGE_NMBR)//Block size in page
//----------------------------------------------------------------------------

//-----------------------Local variables and fucntion-------------------------
UART_HandleTypeDef huart3;
NAND_HandleTypeDef hnand1;

uint32_t _nand_read_8b (NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset);
uint32_t _nand_write_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset);

int fs_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int fs_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
int fs_flash_sync(const struct lfs_config *c);

static uint8_t rd[PAGE_SIZE];
static uint8_t wr[PAGE_SIZE];
//static uint8_t lfs_lookahead_buf[PAGE_SIZE];

static struct lfs_config cfg =
{

    .read_size   = PAGE_SIZE,
    .prog_size   = PAGE_SIZE,
    
    .block_size  = PAGE_NMBR * 2048,
    .block_count = (BLOCK_NMBR * PLANE_NMBR),
    
    .lookahead_size = PAGE_SIZE,
    .cache_size = PAGE_SIZE,
    
	.read_buffer = rd,
	.prog_buffer = wr,
	//.lookahead_buffer = lfs_lookahead_buf,
	//cfg.file_buffer = lfs_file_buf;

    .read = fs_flash_read,
    .prog = fs_flash_prog,
    .erase = fs_flash_erase,
    .sync = fs_flash_sync
};

lfs_file_t file;
lfs_t lfs;

//----------------------------------------------------------------------------

//-----------------------Project options--------------------------------------
//----------------------------------------------------------------------------

//-----------------------Task list--------------------------------------------
void StartDefaultTask (void *pvParameters);
//----------------------------------------------------------------------------

//-----------------------Semaphore list---------------------------------------
//xSemaphoreHandle xbExmpl;
//----------------------------------------------------------------------------

//-----------------------Queue list-------------------------------------------
//xQueueHandle xqMsg;
//----------------------------------------------------------------------------

//-----------------------Programm entry point---------------------------------
int main(void)
{
    //-----------------------HW init----------------------------------------------
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_FSMC_Init();
    MX_USART3_UART_Init();    
    //----------------------------------------------------------------------------

    //-----------------------Creating semaphores----------------------------------
    //SemaphoreCreateBinary(xbExmpl);
    //----------------------------------------------------------------------------

    //-----------------------Creating queues--------------------------------------
    //xqMsg = xQueueCreate(8, sizeof(msg_t));
    //----------------------------------------------------------------------------

    //-----------------------Creating tasks---------------------------------------
    xTaskCreate(StartDefaultTask, "StartDefaultTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    //----------------------------------------------------------------------------

    //-----------------------Semaphores takes-------------------------------------
    //xSemaphoreTake(xbExmpl, portMAX_DELAY);
    //----------------------------------------------------------------------------
    flash_test();

    osKernelStart();
    while (1)
    {
    }
}
//----------------------------------------------------------------------------

//-----------------------StartDefaultTask-------------------------------------
void StartDefaultTask (void *pvParameters)
{
   while(1)
   {
       vTaskDelay(1000);
   }
}
//----------------------------------------------------------------------------

/*-----------------------------------------------------------
/brief: NAND Flash read data
/param: Pointer to NAND handle
/param: Pointer to NAND address
/param: Pointer to buffer
/param: Number bytes to read
/param: Offset in page
/return: 0 if all ok
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
/return: 0 if all ok
-----------------------------------------------------------*/
uint32_t _nand_write_8b(NAND_HandleTypeDef *hnand, NAND_AddressTypeDef *pAddress, uint8_t *pBuffer, uint32_t size, uint32_t offset)
{
    __IO uint32_t index = 0U;
    uint32_t tickstart = 0U;
    uint32_t deviceaddress = 0U, size = 0U, numPagesWritten = 0U, nandaddress = 0U;
  
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
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(Offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
        }
        else /* ((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) > 65535 */
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(Offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_3RD_CYCLE(nandaddress);
        }
    }
    else /* (hnand->Config.PageSize) > 512 */
    {
        if (((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) <= 65535U)
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(Offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_2ND_CYCLE(Offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_1ST_CYCLE(nandaddress);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = ADDR_2ND_CYCLE(nandaddress);
        }
        else /* ((hnand->Config.BlockSize)*(hnand->Config.BlockNbr)) > 65535 */
        {
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_1ST_CYCLE(Offset);
            *(__IO uint8_t *)((uint32_t)(deviceaddress | ADDR_AREA)) = COLUMN_2ND_CYCLE(Offset);
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
/brief: Read data from flash
/param: Pointer to lfs config
/param: Number block
/param: Page offset
/param: Pointer to buffer
/param: Number bytes to read
/return: 0 if all ok
-----------------------------------------------------------*/
int fs_flash_read(  const struct lfs_config *cfg, lfs_block_t block,
                    lfs_off_t off, void *buffer, lfs_size_t size)
{
    assert(off  % cfg->read_size == 0);
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);    
    
    NAND_AddressTypeDef a = {0};
    a.Block = block;
    a.Page = off / hnand1->Config.PageSize;
    _NAND_Read_Page_8b(&hnand1, &a, (uint8_t*)buffer, size, off % hnand1->Config.PageSize);
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
int fs_flash_prog(  const struct lfs_config *cfg, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size)
{

    assert(off  % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);  

    NAND_AddressTypeDef a = {0};
    a.Block = block;
    a.Page = off / hnand1->Config.PageSize;
    _NAND_Write_Page_8b(&hnand1, &a, (uint8_t*)buffer, size, off % hnand1->Config.PageSize);   
    return 0;
}

/*-----------------------------------------------------------
/brief: Erase block
/param: Pointer to lfs config
/param: Number block
/return: 0 if all ok
-----------------------------------------------------------*/
int fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    assert(block < cfg->block_count);  
    NAND_AddressTypeDef a = {0};
    a.Block = block;
    HAL_NAND_Erase_Block(&hnand1, &a);
    return 0;
}

/*-----------------------------------------------------------
/brief: Sync data
/param: Pointer to lfs config
/return: 0 if all ok
-----------------------------------------------------------*/
int fs_flash_sync(const struct lfs_config *c)
{
    return 0;
}

/*-----------------------------------------------------------
/brief: STDIO retarget functions
/param: char to stdio thread
/return: 0 if all ok
-----------------------------------------------------------*/
int stdin_getchar (void)
{
    return 0;
}

int stdout_putchar (int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 1000);
    return 0;
}

int stderr_putchar (int ch)
{ 
    HAL_UART_Transmit(&huart3, (uint8_t*)&ch, 1, 1000);
    return 0;
}
/*---------------------------------------------------------*/