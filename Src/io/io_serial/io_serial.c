#include "io_serial.h"

#include <stdio.h>
#include <string.h>

#include "utils.h"

//---------- HW Specific ---------------------
#include "init_main.h"
#include "stm32f4xx_hal.h"
//---------- HW Specific ---------------------

//-----------------------Local variables and function-------------------------

//---------- HW Specific ---------------------
extern UART_HandleTypeDef   huart3;
//---------- HW Specific ---------------------

// HW Specific function
static void     _hw_init(io_serial_h *ser);
static void     _hw_deinit(io_serial_h *ser);
static void     _hw_set_idle_irq(io_serial_h *ser);
static void     _hw_tx_data(io_serial_h *ser, uint8_t *buf, uint32_t size);
static uint32_t _hw_get_dma_bytes_waiting(io_serial_h *ser);
static void     _hw_set_usart_dma_rx_and_idle_irq(io_serial_h *ser, uint8_t *buf, uint32_t size);
static uint32_t _hw_get_uart_irq_status(io_serial_h *ser);
//----------------------------------------------------------------------------

/*------------------ HW Specific --------------------------
/brief: Init HW
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
static void _hw_init(io_serial_h *ser)
{
    if (ser->type == IO_UART)
    {
        MX_USART3_UART_Init();

        ser->phuart = &huart3;
        ser->phdma  = NULL;
    }
    // TODO: else if (ser->type == IO_USB);
}

/*------------------ HW Specific --------------------------
/brief: DeInit HW
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
static void _hw_deinit(io_serial_h *ser)
{
    if (ser->type == IO_UART)
    {        
        ser->type = IO_NONE;
        
        MX_USART3_UART_Deinit();
    
        ser->phuart = NULL;
        ser->phdma  = NULL;
    }
    // TODO: else if (ser->type == IO_USB);
}

/*------------------ HW Specific --------------------------
/brief: Set IDLE Irq
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
static void _hw_set_idle_irq(io_serial_h *ser)
{
    if (ser->type == IO_UART)
    {
        //---------- HW Specific ---------------------
        UART_HandleTypeDef *p = ser->phuart;
        p->Instance->CR1 |= UART_IT_IDLE;
        //---------- HW Specific ---------------------
    }
    // TODO: else if (ser->type == IO_USB);
}

/*------------------ HW Specific --------------------------
/brief: Serial TX
/param: Pointer to serial handler
/param: Pointer to TX bufer
/param: Number bytes to TX
/return:
-----------------------------------------------------------*/
static void _hw_tx_data(io_serial_h *ser, uint8_t *buf, uint32_t size)
{
    if (ser->type == IO_UART)
    {   
        //---------- HW Specific ---------------------
        UART_HandleTypeDef *p = ser->phuart;
        HAL_UART_Transmit(p, buf, size, 1000);
        //---------- HW Specific ---------------------
    }
    // TODO: else if (ser->type == IO_USB);
}

/*------------------ HW Specific --------------------------
/brief: Get the number of bytes from DMA
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
static uint32_t _hw_get_dma_bytes_waiting(io_serial_h *ser)
{
    if (ser->type == IO_UART)
    {
        //---------- HW Specific ---------------------
        DMA_HandleTypeDef *p = ser->phdma;
        if (p != NULL) return p->Instance->NDTR;
        //---------- HW Specific ---------------------
    }
    // TODO: else if (ser->type == IO_USB);
    return 0;
}

/*------------------ HW Specific --------------------------
/brief: Set DMA rx and IDLE IRQ
/param: Pointer to serial handler
/param: Pointer to RX bufer
/param: Number bytes to RX
/return:
-----------------------------------------------------------*/
static void _hw_set_usart_dma_rx_and_idle_irq(io_serial_h *ser, uint8_t *buf, uint32_t size)
{
    if (ser->type == IO_UART)
    {
        //---------- HW Specific ---------------------
        UART_HandleTypeDef *phuart = ser->phuart;
    
        HAL_UART_DMAStop(phuart); 
        HAL_UART_Receive_DMA(phuart, buf, size);  
        _hw_set_idle_irq(ser);
        //---------- HW Specific ---------------------
    }
    else if (ser->type == IO_USB)
    {
        //Clear header 
        memset((void*)buf, 0, 5);
    }
}

/*------------------ HW Specific --------------------------
/brief: Get UART IRQ status
/param: Pointer to serial handler
/return: IRQ status
-----------------------------------------------------------*/
static uint32_t _hw_get_uart_irq_status(io_serial_h *ser)
{
    uint32_t status = 0;
    
    //---------- HW Specific ---------------------
    UART_HandleTypeDef *p = ser->phuart;
    uint32_t isrflags = UT_READ_REG(p->Instance->SR);

    if (UT_READ_BIT(isrflags, USART_SR_IDLE)) status |= (1 << IDLE_CALLBACK);
    if (UT_READ_BIT(isrflags, USART_SR_TC))   status |= (1 << TX_CALLBACK);
    if (UT_READ_BIT(isrflags, USART_SR_RXNE)) status |= (1 << RX_CALLBACK);
    //---------- HW Specific ---------------------

    return status;
}

/*-----------------------------------------------------------
/brief: Init serial
/param: Pointer to serial handler
/param: Type of serial HW
/return:
-----------------------------------------------------------*/
void io_serial_init(io_serial_h *ser, io_serial_type_h type)
{   
    ser->type = type;
    for (uint32_t i = 0; i < LAST_CALLBACK; i++)
    {
        io_serial_callback_unreg(ser, (io_callback_id_t) i);
    }
    
    _hw_init(ser);
}

/*-----------------------------------------------------------
/brief: DeInit serial
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
void io_serial_deinit(io_serial_h *ser)
{
    _hw_deinit(ser);
}

/*-----------------------------------------------------------
/brief: Get serial type
/param: Pointer to serial handler
/return: Type io_serial
-----------------------------------------------------------*/
io_serial_type_h io_serial_get_type(io_serial_h *ser)
{
    return ser->type;
}

/*-----------------------------------------------------------
/brief: Set Idle IRQ
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
void io_serial_set_idle_irq(io_serial_h *ser)
{
    _hw_set_idle_irq(ser);
}

/*-----------------------------------------------------------
/brief: Get the number of bytes from DMA
/param: Pointer to serial handler
/return:
-----------------------------------------------------------*/
uint32_t io_serial_get_dma_bytes_waiting(io_serial_h *ser)
{
    return _hw_get_dma_bytes_waiting(ser);
}

/*-----------------------------------------------------------
/brief: Serial TX
/param: Pointer to serial handler
/param: Pointer to TX bufer
/param: Number bytes to TX
/return:
-----------------------------------------------------------*/
void io_serial_tx(io_serial_h *ser, uint8_t *buf, uint32_t size)
{
    _hw_tx_data(ser, buf, size);
}

/*-----------------------------------------------------------
/brief: Set DMA rx and IDLE IRQ
/param: Pointer to serial handler
/param: Pointer to RX bufer
/param: Number bytes to RX
/return:
-----------------------------------------------------------*/
void io_serial_set_dma_rx_and_idle_irq(io_serial_h *ser, uint8_t *buf, uint32_t size)
{
    _hw_set_usart_dma_rx_and_idle_irq(ser, buf, size);
}

/*-----------------------------------------------------------
/brief: Register irq callback
/param: Pointer to serial handler
/param: Callback ID
/param: Callback handler
/return: 1 if callback registered, 0 if ID >= LAST_CALLBACK
-----------------------------------------------------------*/
uint32_t io_serial_callback_reg(io_serial_h *ser, io_callback_id_t id, io_callback_handler_t func)
{
    if (id < LAST_CALLBACK)
    {
        ser->callback_list[id] = func;
        return 1;
    }
    return 0;
}

/*-----------------------------------------------------------
/brief: Unregister irq callback
/param: Pointer to serial handler
/param: Callback ID
/param: Callback handler
/return: 1 if callback unregistered, 0 if ID >= LAST_CALLBACK
-----------------------------------------------------------*/
uint32_t io_serial_callback_unreg(io_serial_h *ser, io_callback_id_t id)
{
    if (id < LAST_CALLBACK)
    {
        ser->callback_list[id] = NULL;
        return 1;
    }
    return 0;
}

/*-----------------------------------------------------------
/brief: Call the appropriate callback from IRQ
/param: Pointer to serial handler
/param: Callback ID
/param: Callback handler
/return: 1 if callback unregistered, 0 if ID >= LAST_CALLBACK
-----------------------------------------------------------*/
void io_serial_callback_call(io_serial_h *ser)
{
    uint32_t status = 0;
    if (ser->type == IO_UART)
    {
        status = _hw_get_uart_irq_status(ser);
    }
    // TODO: else if (ser->type == IO_USB);


    for (uint32_t i = 0; i < LAST_CALLBACK; i++)
    {
        if (status & (1 << i))
        {
            if (ser->callback_list[i] != NULL)
            {
                ser->callback_list[i]((void*)ser);
            }
        }
    }    
}
