#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>

#include "init_main.h"

#include "io/io_nand/io_nand.h"
#include "io/io_serial/io_serial.h"
#include "io/io_fs.h"

#include "test.h"



//-----------------------Types and definition---------------------------------
// NOTE: Redefine in stm32f4xx_hal_nand.h for Waveshare board
#define CMD_AREA                   ((uint32_t)(1U<<17U))  /* A16 = CLE high */
#define ADDR_AREA                  ((uint32_t)(1U<<16U))  /* A17 = ALE high */

//#define SPEED_TEST
//#define FIRST_TEST_WRITE_MEASUREMENT
//#define SECOND_TEST_WRITE_MEASUREMENT
#define TEST_READ_MEASUREMENT
//----------------------------------------------------------------------------

//-----------------------Local variables and function-------------------------
io_serial_h _ser;

uint8_t test_data[65536] = {0};
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
    io_serial_init(&_ser, IO_UART);
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

    //test_nand_flash();

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
    
    //------------------------------ Speed test ----------------------------------
#ifdef SPEED_TEST
    // Init test_data array
    for (uint32_t i = 0; i < sizeof(test_data); i++ ) test_data[i] = i;
    
    // Forced formatting
    printf("Format...\r\n");
    err = io_fs_format();
    err = io_fs_mount();
    err = io_fs_file_open(&file, "test_data", IO_FS_O_RDWR | IO_FS_O_CREAT);
    
    // Speed test during the first recording
    #ifdef FIRST_TEST_WRITE_MEASUREMENT
        HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
    #endif
    
    err = io_fs_file_write(&file, &test_data, sizeof(test_data));
    err = io_fs_file_close(&file);
    
    #ifdef FIRST_TEST_WRITE_MEASUREMENT
        HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);
    #endif
    
    err = io_fs_unmount();
    
    //----- Speed test during the second recording ----
    for (uint32_t i = 0; i < sizeof(test_data); i++ ) test_data[i] = 0 - i;
    
    err = io_fs_mount();
    err = io_fs_file_open(&file, "test_data", IO_FS_O_RDWR | IO_FS_O_CREAT);
    
    #ifdef SECOND_TEST_WRITE_MEASUREMENT
        HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
    #endif
    
    err = io_fs_file_rewind(&file);
    err = io_fs_file_write(&file, &test_data, sizeof(test_data));
    // remember the storage is not updated until the file is closed successfully
    err = io_fs_file_close(&file);
    
    #ifdef SECOND_TEST_WRITE_MEASUREMENT
        HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);
    #endif
    
    // release any resources we were using
    err = io_fs_unmount();
    
    //----- Read speed test ----
    err = io_fs_mount();
    err = io_fs_file_open(&file, "test_data", IO_FS_O_RDWR | IO_FS_O_CREAT);
    
    #ifdef TEST_READ_MEASUREMENT
        HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
    #endif
    
    err = io_fs_file_read(&file, &test_data, sizeof(test_data));
    
    #ifdef TEST_READ_MEASUREMENT
        HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);
    #endif

    err = io_fs_file_close(&file);
    err = io_fs_unmount();
#endif    
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
