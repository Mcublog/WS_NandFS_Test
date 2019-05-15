#ifndef IO_SERIAL_H
#define IO_SERIAL_H

#include <stdint.h>

#include "io_serial_data_type.h"

void io_serial_init(io_serial_h *ser, io_serial_type_h type);
void io_serial_deinit(io_serial_h *ser);

io_serial_type_h    io_serial_get_type(io_serial_h *ser);
void                io_serial_set_idle_irq(io_serial_h *ser);

void io_serial_tx(io_serial_h *ser, uint8_t *buf, uint32_t size);

uint32_t io_serial_get_dma_bytes_waiting(io_serial_h *ser);
void     io_serial_set_dma_rx_and_idle_irq(io_serial_h *ser, uint8_t *buf, uint32_t size);

uint32_t io_serial_callback_reg(io_serial_h *ser, io_callback_id_t id, io_callback_handler_t func);
uint32_t io_serial_callback_unreg(io_serial_h *ser, io_callback_id_t id);
void     io_serial_callback_call(io_serial_h *ser);

#endif // IO_SERIAL_H
