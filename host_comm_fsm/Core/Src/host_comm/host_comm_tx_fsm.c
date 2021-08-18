/**
 * @file host_comm_tx_fsm.c
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief 
 * @version 0.1
 * @date 2020-06-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "host_comm_tx_fsm.h"

/*@brief Tx comm finite state machine object */
host_tx_comm_fsm_t host_tx_comm_handle;


/**@brief Enable/Disable debug messages */
#define HOST_TX_FSM_DEBUG 0
#define HOST_TX_FSM_TAG "host tx comm : "

/**@brief uart debug function for server comm operations  */
#if HOST_TX_FSM_DEBUG
#define host_tx_comm_dbg_msg(format, ...) printf(HOST_TX_FSM_TAG format, ##__VA_ARGS__)
#else
#define host_tx_comm_dbg_msg(format, ...) \
    do                                    \
    { /* Do nothing */                    \
    } while (0)
#endif

/*Static functions for state poll pending transfers */
static void enter_seq_poll_pending_transfers(host_tx_comm_fsm_t *handle);
static void exit_action_poll_pending_transfers(host_tx_comm_fsm_t *handle);
static void during_action_poll_pending_transfers(host_tx_comm_fsm_t *handle);
static bool poll_pending_transfers_on_react(host_tx_comm_fsm_t *handle, const bool try_transition);

/*Static functions for state transmit packet*/
static void enter_seq_transmit_packet(host_tx_comm_fsm_t *handle);
static void entry_action_transmit_packet(host_tx_comm_fsm_t *handle);
static void exit_action_transmit_packet(host_tx_comm_fsm_t *handle);
static bool transmit_packet_on_react(host_tx_comm_fsm_t *handle, const bool try_transition);

/*Static methods of the finite state machine*/
static void tx_send_packet(host_tx_comm_fsm_t *handle);
static uint8_t get_crc(uint8_t *packet, uint8_t len);


static void clear_events(host_tx_comm_fsm_t* handle)
{
    handle->event.internal = ev_int_tx_comm_invalid;
    handle->event.external = ev_ext_tx_comm_invalid;
    time_event_stop(&handle->event.time.ack_timeout);
}


static void host_tx_comm_fsm_set_next_state(host_tx_comm_fsm_t *handle, host_tx_comm_states_t next_state)
{
	handle->state = next_state;
    clear_events(handle);
}

void host_tx_comm_fsm_init(host_tx_comm_fsm_t* handle)
{
    /*Init interface*/
    host_comm_tx_queue_init();
    memset((uint8_t*)&handle->iface.request.packet, 0, sizeof(packet_frame_t));

    /*Clear events */
    clear_events(handle);

    /*defaut enter sequence */
    enter_seq_poll_pending_transfers(handle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void enter_seq_poll_pending_transfers(host_tx_comm_fsm_t *handle)
{
	host_tx_comm_dbg_msg("enter seq \t[ poll_pending_transfers ]\n\r");
	host_tx_comm_fsm_set_next_state(handle, st_tx_comm_poll_pending_transfer);
    handle->iface.retry_cnt = 0;
}


static void during_action_poll_pending_transfers(host_tx_comm_fsm_t *handle)
{
    if(host_comm_tx_queue_get_pending_transfers())
    {
        handle->event.internal = ev_int_tx_comm_pending_packet;
    }
}


static void exit_action_poll_pending_transfers(host_tx_comm_fsm_t *handle)
{
    /*Read packet to transfer */
    host_comm_tx_queue_read_request(&handle->iface.request);
}


static bool poll_pending_transfers_on_react(host_tx_comm_fsm_t *handle, const bool try_transition)
{
	/* The reactions of state 'check preamble' */
	bool did_transition = try_transition;

	if (try_transition == true)
	{
		if (handle->event.internal == ev_int_tx_comm_pending_packet)
		{
            /*Exit action */
            exit_action_poll_pending_transfers(handle);
			/*Enter sequence */
			enter_seq_transmit_packet(handle);
		}
		else
			did_transition = false;
	}
	if ((did_transition) == (false))
	{
		/*during action*/
		during_action_poll_pending_transfers(handle);
	}
	return did_transition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void enter_seq_transmit_packet(host_tx_comm_fsm_t *handle)
{
	host_tx_comm_dbg_msg("enter seq \t[ transmit_packet ]\n\r");
	host_tx_comm_fsm_set_next_state(handle, st_tx_comm_transmit_packet);
    entry_action_transmit_packet(handle);
}

static void entry_action_transmit_packet(host_tx_comm_fsm_t *handle)
{
    if(handle->iface.request.ack_response == true)
    {
        time_event_start(&handle->event.time.ack_timeout, MAX_ACK_TIMEOUT_MS);
    }
    else
    {
        handle->event.internal = ev_int_tx_comm_no_ack_required;
    }
    tx_send_packet(handle);
}

static void exit_action_transmit_packet(host_tx_comm_fsm_t *handle)
{
    time_event_stop(&handle->event.time.ack_timeout);
}


static bool transmit_packet_on_react(host_tx_comm_fsm_t *handle, const bool try_transition)
{
    /* The reactions of state 'check preamble' */
    bool did_transition = try_transition;

    if (try_transition == true)
    {
        if ((handle->event.external == ev_ext_tx_comm_ack_received) |
            (handle->event.internal == ev_int_tx_comm_no_ack_required))
        {
            exit_action_transmit_packet(handle);
            enter_seq_poll_pending_transfers(handle);
        }

        else if(handle->event.external == ev_ext_tx_comm_nack_received)
        {
            exit_action_transmit_packet(handle);
            enter_seq_transmit_packet(handle);
        }

        else if (time_event_is_raised(&handle->event.time.ack_timeout) == true)
        {
            exit_action_transmit_packet(handle);

            /*Enter sequence */
            if (handle->iface.retry_cnt++ >= MAX_NUM_OF_TRANSFER_RETRIES) 
                enter_seq_poll_pending_transfers(handle);
            else
                enter_seq_transmit_packet(handle);
        }

        else
            did_transition = false;
    }
    if ((did_transition) == (false))
    {
        
    }
    return did_transition;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief           calculate the CRC value of an specific frame.
 * @param frame     pointer to frame to be processed.   
 * @param len       length in bytes of the frame.
 * @return uint8_t  return CRC value of the frame 
 */
static uint32_t frame_get_crc(uint8_t *frame, uint8_t frame_len)
{

}

/**
 * @brief              check if a calculated CRC value of a @frame of length @frame_len is equal to the expected
 *                     CRC value @exp_crc
 * @param frame        pointer to frame to be processed. 
 * @param frame_len    length in bytes of the frame.
 * @param exp_crc  expected CRC value of the frame. 
 * @return uint8_t     returns 1 if the expected CRC value is equal to the calculated CRC of the frame.
 */

static uint8_t frame_check_crc(uint8_t *frame, uint8_t frame_len, uint32_t exp_crc)
{
    /*Get CRC value from frame */
    uint8_t packet_crc = frame_get_crc(frame, frame_len);

    /*Compare calculated CRC and buffer CRC */
    if (packet_crc == exp_crc)
        return 1;
    else
    	server_comm_dbg_message("crc error : crc [0x%X] != exp_crc [0x%X] \r\n", packet_crc, exp_crc);

    return 0;
}


/**
 * @brief 
 * 
 * @param host_comm 
 * @param packet 
 * @return uint8_t 
 */
static void tx_send_packet(host_tx_comm_fsm_t *handle)
{
   /* packet index to write bytes  */
    uint16_t preamble = PREAMBLE;
    uint16_t postamble = POSTAMBLE;
    uint32_t crc = 0;

    packet_data_t *packet = &handle->iface.request.packet;

    /* Transmit preamble */
    if (!uart_transmit_it((uint8_t *)&preamble, sizeof(preamble)))
        return 0;

    /* Start CRC calculation*/
    crc = frame_get_crc((uint8_t *)&packet->header, HEADER_SIZE_BYTES);

    /* Transmit Header */
    if (!uart_transmit_it((uint8_t *)&packet->header, HEADER_SIZE_BYTES))
        return 0;

    /* If Payload  */
    if (packet->header.payload_len)
    {
        /*update CRC*/
        crc ^= frame_get_crc((uint8_t *)&packet->payload, packet->header.payload_len);

        /*Transmit payload*/
        if (!uart_transmit_it((uint8_t *)&packet->payload, packet->header.payload_len))
            return 0;
    }

    /*Transmit CRC*/
    if (!uart_transmit_it((uint8_t *)&crc, CRC_SIZE_BYTES))
        return 0;

    /*Transmit Postamble*/
    if (!uart_transmit_it((uint8_t *)&postamble, POSTAMBLE_SIZE_BYTES))
        return 0;

    return 1;
}



uint8_t host_tx_comm_fsm_write_dbg_msg(host_tx_comm_fsm_t *handle, char *dbg_msg)
{
	/* Check frame identifier */
	if (dbg_msg != NULL)
	{
		/*form header*/
		tx_request_t request;
        request.ack_response = true;
        request.src = TX_SRC_FW_USER;
		request.packet.header.dir = TARGET_TO_HOST_DIR;
		request.packet.header.type.evt = TARGET_TO_HOST_EVT_PRINT_DBG_MSG;
		request.packet.header.payload_len = strlen(dbg_msg);

		/*copy dbg message to payload*/
        if((request.packet.header.payload_len > 0) && (request.packet.header.payload_len < MAX_PAYLOAD_SIZE))
		    memcpy((uint8_t*)&request.packet.payload, dbg_msg, request.packet.header.payload_len);
        else
            return 0;

		/*Write Data*/
        return host_comm_tx_queue_write_request(&request);
	}

	return 0;
}


void host_tx_comm_fsm_time_event_update(host_tx_comm_fsm_t *handle)
{
	time_event_t *time_event = (time_event_t *)&handle->event.time;
	for (int tev_idx = 0; tev_idx < sizeof(handle->event.time) / sizeof(time_event_t); tev_idx++)
	{
		time_event_update(time_event);
		time_event++;
	}
}

void host_tx_comm_fsm_run(host_tx_comm_fsm_t *handle)
{
    switch (handle->state)
    {
    case st_tx_comm_poll_pending_transfer:
        poll_pending_transfers_on_react(handle, true);
        break;
    case st_tx_comm_transmit_packet:
        transmit_packet_on_react(handle, true);
        break;
    default:
        break;
    }
}

void host_tx_comm_fsm_set_ext_event(host_tx_comm_fsm_t* handle, host_tx_comm_external_events_t event)
{
    handle->event.external = event;
}

