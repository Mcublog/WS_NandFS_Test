#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

#include "init_main.h"
#include "test.h"
#include "io_nand.h"
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
    io_nand_write_8b(block + off, (uint8_t*) buffer, size, off % hnand1->Config.PageSize);
    //_NAND_Read_Page_8b(&hnand1, &a, (uint8_t*)buffer, size, off % hnand1->Config.PageSize);
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
    io_nand_read_8b (block + off, (uint8_t*) buffer, size, off % hnand1->Config.PageSize);
    //_NAND_Write_Page_8b(&hnand1, &a, (uint8_t*)buffer, size, off % hnand1->Config.PageSize);   
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
    io_nand_erase(block);
    //HAL_NAND_Erase_Block(&hnand1, &a);
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