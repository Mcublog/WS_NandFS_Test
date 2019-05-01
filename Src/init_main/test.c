#include "test.h"

#include <stdint.h>
#include <stdio.h>

#include "init_main.h"
#include "io_nand.h"

#define PAGE_SIZE       (2048)//Page z
#define SPARE_SIZE      (64)//область spare в байтах
#define BLOCK_SIZE      (64)//размер блока в страницах
#define BLOCK_NUMBER    (1024 * PLANE_NUMBER)//общее кол-во блоков
#define PLANE_NUMBER    (1)//кол-во плейнов
#define PLANE_SIZE      (1024)//размер плейна в блоках

void flash_test(void)
{
	uint32_t i = 0;
	uint32_t a = 0;
    
	uint8_t buff[PAGE_SIZE];
    	   
    io_nand_erase(a);
	
    io_nand_read_8b (a, (uint8_t*) &buff, PAGE_SIZE, 0);
    
	for (i = 0; i < PAGE_SIZE; i++) buff[i] = i;
    
    printf("----------------\r\n");  
    printf("Write test buff:\r\n");
	for (i = 0; i < PAGE_SIZE; i++)
    {
        buff[i] = i;
        printf("%#x ", buff[i]);
        if (i % 16 == 0 && i != 0) printf("\r\n");
    }
    printf("\r\n----------------\r\n\r\n");   
    io_nand_write_8b(a, (uint8_t*)&buff, PAGE_SIZE, 0);
    
	for (i = 0; i < PAGE_SIZE; i++) buff[i]=0;
    io_nand_read_8b (a, (uint8_t*) &buff, PAGE_SIZE, 0);
    
    printf("----------------\r\n"); 
    printf("Read test buff and check:\r\n");
	for (i = 0; i < PAGE_SIZE; i++)
    {
        if (buff[i] != i % 256)
        {
            printf("Error at: %d\r\n", i);
            break;
        }
        printf("%#x ", buff[i]);
        if (i % 16 == 0 && i != 0) printf("\r\n");
    }
    printf("\r\n----------------\r\n\r\n");
    
}
//#endif
