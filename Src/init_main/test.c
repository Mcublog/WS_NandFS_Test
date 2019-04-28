#include "test.h"

#include <stdint.h>
#include <stdio.h>

#include "init_main.h"

extern NAND_HandleTypeDef hnand1;

#define PAGE_SIZE       (2048)//размер страницы в байтах
#define SPARE_SIZE      (64)//область spare в байтах
#define BLOCK_SIZE      (64)//размер блока в страницах
#define BLOCK_NUMBER    (1024 * PLANE_NUMBER)//общее кол-во блоков
#define PLANE_NUMBER    (1)//кол-во плейнов
#define PLANE_SIZE      (1024)//размер плейна в блоках

static void _adr_init(NAND_AddressTypeDef* adr)
{
	adr->Page=0;
	adr->Block=0;
	adr->Plane=0;
}

void flash_test(void)
{
	uint32_t i=0;
	NAND_AddressTypeDef a = {0};
    _adr_init(&a);
    NAND_IDTypeDef nand_id;
	uint8_t buff[SPARE_SIZE];
	uint8_t buff_main[PAGE_SIZE];
    	
	HAL_NAND_Reset(&hnand1);
    HAL_NAND_Read_ID(&hnand1, &nand_id);
    
	HAL_NAND_Erase_Block(&hnand1, &a);
	
	HAL_NAND_Read_Page_8b(&hnand1, &a, (uint8_t*)&buff_main, 1);
    
	for (i = 0; i < PAGE_SIZE; i++) buff_main[i] = i;
    
    printf("----------------\r\n");  
    printf("Write test buff:\r\n");
	for (i = 0; i < PAGE_SIZE; i++)
    {
        buff_main[i] = i;
        printf("%#x ", buff_main[i]);
        if (i % 16 == 0 && i != 0) printf("\r\n");
    }
    printf("\r\n----------------\r\n\r\n");   
    
	_adr_init(&a);
	HAL_NAND_Write_Page_8b(&hnand1, &a, (uint8_t*)&buff_main, 1);
	_adr_init(&a);
	for (i = 0; i < PAGE_SIZE; i++) buff_main[i]=0;
	HAL_NAND_Read_Page_8b(&hnand1, &a, (uint8_t*)&buff_main, 1);
    
    printf("----------------\r\n"); 
    printf("Read test buff:\r\n");
	for (i = 0; i < PAGE_SIZE; i++)
    {
        printf("%#x ", buff_main[i]);
        if (i % 16 == 0 && i != 0) printf("\r\n");
    }
    printf("\r\n----------------\r\n\r\n");
    
	
	HAL_NAND_Read_SpareArea_8b(&hnand1, &a, (uint8_t*)&buff, 1);
	_adr_init(&a);
	for (i = 0; i < SPARE_SIZE; i++) buff[i]=i;
    
    printf("----------------\r\n");  
    printf("Write test buff:\r\n");
	for (i = 0; i < SPARE_SIZE; i++)
    {
        printf("%#x ", buff[i]);
        if (i % 16 == 0 && i != 0) printf("\r\n");
    }
    printf("\r\n----------------\r\n\r\n"); 
    
	HAL_NAND_Write_SpareArea_8b(&hnand1, &a, (uint8_t*)&buff, 1);
	_adr_init(&a);
	for (i = 0; i < SPARE_SIZE; i++) buff[i]=0;
	HAL_NAND_Read_SpareArea_8b(&hnand1, &a, (uint8_t*)&buff, 1);
    
    printf("----------------\r\n"); 
    printf("Read test buff:\r\n");
	for (i = 0;i < SPARE_SIZE; i++)
    {
        printf("%#x ",buff[i]);
        if (i % 16 == 0 && i != 0) printf("\r\n");
    }
    printf("\r\n----------------\r\n\r\n");
}
//#endif
