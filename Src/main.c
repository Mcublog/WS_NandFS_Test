#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

#include "init_main.h"
#include "test.h"
#include "io_nand.h"
#include "io_fs.h"

//-----------------------Types and definition---------------------------------
// NOTE: Redefine in stm32f4xx_hal_nand.h for Waveshare board
#define CMD_AREA                   ((uint32_t)(1U<<17U))  /* A16 = CLE high */
#define ADDR_AREA                  ((uint32_t)(1U<<16U))  /* A17 = ALE high */
//----------------------------------------------------------------------------

//-----------------------Local variables and fucntion-------------------------
UART_HandleTypeDef huart3;
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
    MX_USART3_UART_Init();    
    io_nand_init();
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
    io_fs_file file;
    volatile int32_t err = io_fs_init();
    
    err = io_fs_mount();
    if (err < 0)    
    {
        printf("Format...\r\n");
        err = io_fs_format();
        err = io_fs_mount();
    }
    
    // read current count
    uint32_t boot_count = 0;
    err = io_fs_file_open(&file, "boot_count", IO_FS_O_RDWR | IO_FS_O_CREAT);
    err = io_fs_file_read(&file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    err = io_fs_file_rewind(&file);
    err = io_fs_file_write(&file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    err = io_fs_file_close(&file);

    // release any resources we were using
    err = io_fs_unmount();

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
