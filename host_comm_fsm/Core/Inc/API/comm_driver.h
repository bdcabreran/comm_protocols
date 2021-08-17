#ifndef COMM_DRIVER_H
#define COMM_DRIVER_H

#include "circular_buffer.h"
#include "stm32f4xx_hal.h"

#define MAX_DATA_CHUNK_SIZE (100)

uint8_t host_comm_init(uint8_t *rx_buff, uint16_t rx_buff_len,
                       uint8_t *tx_buff, uint16_t tx_buff_len);
uint8_t host_comm_get_rx_data_len(void);
uint8_t host_comm_read_rx_data(uint8_t *data, uint8_t len);
uint8_t host_comm_fetch_rx_data(uint8_t *data, uint8_t len);
uint8_t host_comm_clear_rx_data(void);
uint8_t host_comm_transmit(uint8_t *data, uint8_t len);
uint8_t host_comm_transmit_it(uint8_t *data, uint8_t len);


#endif