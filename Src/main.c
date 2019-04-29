#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

#include "init_main.h"
#include "test.h"
#include "io_nand.h"
#include "io_fs.h"
#include "lfs.h"

//-----------------------Types and definition---------------------------------
// NOTE: Redefine in stm32f4xx_hal_nand.h for Waveshare board
#define CMD_AREA                   ((uint32_t)(1U<<17U))  /* A16 = CLE high */
#define ADDR_AREA                  ((uint32_t)(1U<<16U))  /* A17 = ALE high */
//----------------------------------------------------------------------------

//-----------------------Local variables and fucntion-------------------------
UART_HandleTypeDef huart3;
NAND_HandleTypeDef hnand1;
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

    //flash_test();

    //----------------------------------------------------------------------------
    io_nand_init_cfg();
    const uint32_t page_size = io_nand_get_page_size();

    uint8_t rd[2048];
    uint8_t wr[2048];
    
    struct lfs_config cfg =
    {

        .read_size   = page_size,
        .prog_size   = page_size,
    
        .block_size  = io_nand_get_block_size() * page_size,
        .block_count = io_nand_get_block_number(),
    
        .lookahead_size = page_size,
        .cache_size     = page_size,
    
	    .read_buffer = rd,
	    .prog_buffer = wr,

        .read   = io_fs_flash_read,
        .prog   = io_fs_flash_prog,
        .erase  = io_fs_flash_erase,
        .sync   = io_fs_flash_sync
    };

    lfs_file_t  file;
    lfs_t       lfs;
    
    volatile int32_t err = 0;
    err = lfs_mount(&lfs, &cfg);
    if (err < 0)
    {
        err = lfs_format(&lfs, &cfg);
        err = lfs_mount(&lfs, &cfg);
    }
    
    // read current count
    uint32_t boot_count = 0;
    err = lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    err = lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    err = lfs_file_rewind(&lfs, &file);
    err = lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    err = lfs_file_close(&lfs, &file);

    // release any resources we were using
    err = lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
    //----------------------------------------------------------------------------

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
