#ifndef IO_SERIAL_DATA_TYPE_H
#define IO_SERIAL_DATA_TYPE_H

#include <stdint.h>

typedef enum
{
    IO_NONE,    
    IO_UART,
    IO_USB,
}io_serial_type_h;

typedef enum
{
    TX_CALLBACK = 0,
    RX_CALLBACK,
    IDLE_CALLBACK,
//-----------------    
    LAST_CALLBACK
}io_callback_id_t;

typedef void (*io_callback_handler_t)(void *context);

typedef struct
{
    io_serial_type_h type;
    io_callback_handler_t callback_list[LAST_CALLBACK];
    void *phuart;
    void *phdma;
}io_serial_h;

#endif // IO_SERIAL_DATA_TYPE_H
