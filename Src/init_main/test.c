#include "test.h"

#include <stdint.h>
#include <stdio.h>

#include "init_main.h"
#include "io_nand.h"

#define PAGE_SIZE       (2048)

void test_nand_flash(void)
{
	uint32_t i = 0;
	uint32_t a = 0;
    
	uint8_t buff[PAGE_SIZE];
    	   
    io_nand_block_erase(a);	
    io_nand_read (a, (uint8_t*) &buff, PAGE_SIZE, 0);
    
    printf("----------------\r\n"); 
    printf("Erase test\r\n");    
    for (i = 0; i < PAGE_SIZE; i++)
    {
        if (buff[i] != 0xFF)// all characters must be 0xFF
        {
            printf("Error erase test at: %d\r\n", i);
            return;            
        }
    }
    printf("Erase check: OK...\r\n");    
    printf("----------------\r\n\r\n");
    
    for (a = 1 * PAGE_SIZE; a < 5 * PAGE_SIZE; a = a + PAGE_SIZE)
    {
        printf("Test addres: %#x\r\n", a);
        for (i = 0; i < PAGE_SIZE; i++) buff[i] = i;
    
        printf("----------------\r\n");  
        printf("Write buff...\r\n");
        for (i = 0; i < PAGE_SIZE; i++)
        {
            buff[i] = i;
            //printf("%#x ", buff[i]);
            //if (i % 16 == 0 && i != 0) printf("\r\n");
        }
        io_nand_write(a, (uint8_t*)&buff, PAGE_SIZE, 0);
        printf("Write buffer: OK...\r\n");
        printf("----------------\r\n\r\n");
    
        for (i = 0; i < PAGE_SIZE; i++) buff[i]=0;
        io_nand_read (a , (uint8_t*) &buff, PAGE_SIZE, 0);
    
        printf("----------------\r\n"); 
        printf("Read buff and check...\r\n");
        for (i = 0; i < PAGE_SIZE; i++)
        {
            if (buff[i] != i % 256)
            {
                printf("Error read at: %d\r\n", i);
                return;
            }
            //printf("%#x ", buff[i]);
            //if (i % 16 == 0 && i != 0) printf("\r\n");
        }
        printf("Read check: OK...\r\n");
        printf("----------------\r\n\r\n");
    }
    

    
}
//#endif
