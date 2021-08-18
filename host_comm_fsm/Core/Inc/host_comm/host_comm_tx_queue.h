/**
 * @file host_comm_tx_queue.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief  Host communication transmission queue
 * @version 0.1
 * @date 2021-08-18
 * 
 */

#ifndef HOST_COMM_TX_QUEUE
#define HOST_COMM_TX_QUEUE

#include "protocol.h"
#include "stdint.h"
#include "circular_buffer.h"
#include "stdbool.h"

#define TX_QUEUE_BUFF_SIZE       (1024)

/**
 * @brief Enumeration of the process source that request a transmission
 * 
 */
typedef enum
{
    TX_SRC_REQ_INVALID,
    TX_SRC_HOST_COMM_RX_FSM,
    TX_SRC_REQ_LAST
}tx_request_source_t;

typedef struct
{
    tx_request_source_t src; /* process that request a transmission*/
    packet_data_t packet;     /* packet data to be transmitted */
    bool ack_response;       /* ACK response expected ? */      

}tx_request_t;

uint8_t host_comm_write_request_in_tx_queue(tx_request_t *tx_request);
uint8_t host_comm_read_request_from_tx_queue(tx_request_t *tx_request);
uint8_t host_comm_fetch_request_from_tx_queue(tx_request_t *tx_request);

#endif
