/**
 * @file host_comm_tx_queue.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief  Host communication transmission queue
 * @version 0.1
 * @date 2021-08-18
 * 
 */


#include "host_comm_tx_queue.h"

/*Enable/Disable Debug messages*/
#define HOST_COMM_TX_DEBUG 0
#define HOST_COMM_TX_TAG "tx queue: "

/**@brief uart debug function for Tx comm operations  */
#if HOST_COMM_TX_DEBUG
#define hdx_comm_dbg_message(format, ...) printf(HOST_COMM_TX_TAG format, ##__VA_ARGS__)
#else
#define hdx_comm_dbg_message(format, ...) \
    do                                    \
    { /* Do nothing */                    \
    } while (0)
#endif


typedef struct
{ 
    size_t packet_cnt;                   /*!< counter that stores the number of pending transmission packets in the Tx queue*/       
    c_buff_handle_t  cb;                 /*!< circular buffer that stores the packet data yo be transmit  */
    uint8_t buffer[TX_QUEUE_BUFF_SIZE];  /*!< buffer to store the data to be transmitted */     
}host_comm_tx_queue_t;

static host_comm_tx_queue_t tx_queue;

/**
 * @brief  Initializes Tx communication queue to handle packet transmission read/write operations. 
 * 
 * @param tx_queue Pointer to a variable of type @tx_comm_queue_t Tx communication queue struct
 */
void host_comm_tx_init_queue(void)
{
    tx_queue.cb = circular_buff_init(tx_queue.buffer, TX_QUEUE_BUFF_SIZE);
    tx_queue.packet_cnt = 0;
}

/**
 * @brief Write packet in Tx communication queue
 * 
 * @param packet Pointer to a variable of type @packet_data_t packet to be write in queue.
 * @return uint8_t Returns 1 if the packet has been written successfully, return 0 otherwise.
 */
uint8_t host_comm_write_request_in_tx_queue(tx_request_t *tx_request)
{
    /* Temporal variable to check free space needed to write packet in tx queue */
    uint8_t packet_data_len = HEADER_SIZE_BYTES + tx_request->packet.header.payload_len;


    if (circular_buff_get_free_space(tx_queue.cb) > packet_data_len + 1) //include byte for req src 
    {
        circular_buff_put(tx_queue.cb, (uint8_t)tx_request->src);
        circular_buff_write(tx_queue.cb, (uint8_t *)&tx_request->packet, packet_data_len);
        tx_queue.packet_cnt++;

        hdx_comm_dbg_message("pending packet counter [%d]\r\n", tx_queue.packet_cnt);
        hdx_comm_dbg_message("free space in queue [%d] bytes\r\n", circular_buff_get_free_space(tx_queue.cb));

        return 1;
    }
    else
    {
        hdx_comm_dbg_message("not enough space in tx queue ");
        return 0;
    }
}

/**
 * @brief  Read a packet from Tx communication queue and save it in packet pointer entry
 * 
 * @param request_src    Retrives the process source that solicitate the transmission 
 * @param packet         Pointer to a variable of type @packet_data_t, packet to be stored the data read from queue.
 * @return uint8_t       Returns 1 if there is an available packet to be read, returns 0 otherwise.
 */
uint8_t host_comm_read_request_from_tx_queue(tx_request_t *tx_request)
{
    if (tx_queue.packet_cnt > 0)
    {
        circular_buff_get(tx_queue.cb, (uint8_t *)tx_request->src);
        circular_buff_read(tx_queue.cb, (uint8_t *)&tx_request->packet.header, HEADER_SIZE_BYTES);
        circular_buff_read(tx_queue.cb, (uint8_t *)&tx_request->packet.payload, tx_request->packet.header.payload_len);
        tx_queue.packet_cnt--;

        return 1;
    }
    else
    {
        hdx_comm_dbg_message("error there are not pending transfers");
        return 0;
    }
}

/*
 * @brief               Fetch request packet header in Tx communication queue.
 * 
 * @param tx_queue            Pointer to a variable of type @tx_comm_queue_t Tx communication queue struct 
 * @param request_pkt_header  Pointer to a variable of type @gateway_to_wristband_header_t which contains the header parameters 
 *                            of a request packet to wristband. 
 * @return uint8_t            Returns 1 if there is an available packet to be fetch, returns 0 otherwise.
 */
uint8_t host_comm_fetch_request_from_tx_queue(tx_request_t *tx_request)
{
    if (tx_queue.packet_cnt > 0)
    {
        circular_buff_fetch(tx_queue.cb, (uint8_t *)tx_request->src); 
        circular_buff_fetch(tx_queue.cb, (uint8_t *)&tx_request->packet.header, HEADER_SIZE_BYTES);
        circular_buff_fetch(tx_queue.cb, (uint8_t *)&tx_request->packet.payload, tx_request->packet.header.payload_len);
        return 1;
    }
    return 0;
}