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
	assert(IS_HOST_TX_COMM_STATE(next_state));
	handle->state = next_state;
    clear_events(handle);
}

void host_tx_comm_fsm_init(host_tx_comm_fsm_t* handle)
{
    /*Init interface*/
    memset((uint8_t*)&handle->iface.tx.packet, 0, sizeof(packet_frame_t));
    handle->iface.tx.c_buff = circular_buff_init((uint8_t*)&handle->iface.tx.buff, TX_CIRCULAR_BUFF_SIZE);
    handle->iface.tx.retry_cnt = 0;

    /*Clear events */
    clear_events(handle);

    /*defaut enter sequence */
    enter_seq_poll_pending_transfers(handle);
}

static void enter_seq_poll_pending_transfers(host_tx_comm_fsm_t *handle)
{
	host_tx_comm_dbg_msg("enter seq \t[ poll_pending_transfers ]\n\r");
	host_tx_comm_fsm_set_next_state(handle, st_tx_comm_poll_pending_transfer);
    handle->iface.tx.retry_cnt = 0;
}


static void during_action_poll_pending_transfers(host_tx_comm_fsm_t *handle)
{
    if(circular_buff_get_data_len(handle->iface.tx.c_buff) > sizeof(packet_header_t))
    {
        handle->event.internal = ev_int_tx_comm_pending_packet;
    }
}


static void exit_action_poll_pending_transfers(host_tx_comm_fsm_t *handle)
{
  /*Read packet to transfer */
    packet_frame_t *packet = &handle->iface.tx.packet;

    circular_buff_read(handle->iface.tx.c_buff, (uint8_t*)&packet->header, sizeof(packet_header_t));
    if(packet->header.payload_len)
        circular_buff_read(handle->iface.tx.c_buff, (uint8_t*)&packet->payload, packet->header.payload_len);
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
    time_event_start(&handle->event.time.ack_timeout, MAX_ACK_TIMEOUT, false);
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
        if (handle->event.external == ev_ext_tx_comm_ack_received)
        {
            /*Exit action */
            exit_action_transmit_packet(handle);
            /*Enter sequence */
            enter_seq_poll_pending_transfers(handle);
        }
        else if(handle->event.external == ev_ext_tx_comm_nack_received)
        {
            /*Exit action */
            exit_action_transmit_packet(handle);
            /*Enter sequence */
            enter_seq_transmit_packet(handle);
        }
        else if (time_event_is_raised(&handle->event.time.ack_timeout) == true)
        {
            /*Exit action */
            exit_action_transmit_packet(handle);

            /*Enter sequence */
            if (handle->iface.tx.retry_cnt++ >= MAX_NUM_OF_TRANSFER_RETRIES) 
                enter_seq_poll_pending_transfers(handle);
            else
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

/**
 * @brief 
 * 
 * @param host_comm 
 * @param packet 
 * @return uint8_t 
 */
static void tx_send_packet(host_tx_comm_fsm_t *handle)
{
    /*Pointer to packet */
    uint8_t packet_offset = 0;
    uint16_t preamble = PREAMBLE;
    uint8_t  postamble = POSTAMBLE;
    uint8_t temp_buff[PREAMBLE_SIZE + PACKET_HEADER_SIZE + PAYLOAD_BUFF_SIZE + CRC_SIZE + POSTAMBLE_SIZE] = {0};

    packet_frame_t *packet = &handle->iface.tx.packet;

    memcpy((uint8_t*)temp_buff, (uint8_t*)&preamble, PREAMBLE_SIZE);
    packet_offset += PREAMBLE_SIZE;

    memcpy((uint8_t*)temp_buff + packet_offset, (uint8_t*)&packet->header, PACKET_HEADER_SIZE);
    packet_offset += PACKET_HEADER_SIZE;

    if(packet->header.payload_len)
    {
        memcpy((uint8_t*)temp_buff + packet_offset, (uint8_t*)&packet->payload, packet->header.payload_len);
        packet_offset += packet->header.payload_len;   
    }

    uint8_t crc = get_crc((uint8_t *)&packet->header, sizeof(packet_header_t));

    if (packet->header.payload_len)
    {
        crc ^= get_crc((uint8_t *)&packet->payload, packet->header.payload_len);
    }

    memcpy((uint8_t*)temp_buff + packet_offset, (uint8_t*)&crc, CRC_SIZE);
    packet_offset += CRC_SIZE;

    memcpy((uint8_t*)temp_buff + packet_offset, (uint8_t*)&postamble, POSTAMBLE_SIZE);
    packet_offset += POSTAMBLE_SIZE;

    /*Send Data*/
    usb_vcp_transmit_data((char *)&temp_buff, packet_offset);
}

uint8_t host_tx_comm_fsm_write_packet(host_tx_comm_fsm_t *handle, packet_frame_t *packet)
{
    /* Check frame identifier */
    if (circular_buff_get_free_space(handle->iface.tx.c_buff) > PACKET_HEADER_SIZE + packet->header.payload_len)
    {
        if (packet->header.frame_id == TEST_JIG_TO_HOST)
        {
            /* Check if device is valid */
            if (IS_VALID_DEVICE_ID(packet->header.device_id))
            {
                /*Check if request or command is in range*/
                if (IS_JIG_TO_HOST_CMD(packet->header.cmd) || (IS_JIG_TO_HOST_RESP(packet->header.resp)))
                {
                    /*Write Data*/
                    circular_buff_write(handle->iface.tx.c_buff, (uint8_t *)&packet->header, sizeof(packet_header_t));

                    if (packet->header.payload_len)
                        circular_buff_write(handle->iface.tx.c_buff, (uint8_t *)&packet->payload, packet->header.payload_len);
                    return 1;
                }
            }
        }
    }

    return 0;
}

uint8_t host_tx_comm_fsm_write_test_result(host_tx_comm_fsm_t *handle, ib_device_t device, ib_test_result_t *test_result)
{
	if (IS_VALID_DEVICE_ID(device) && IS_VALID_CHANNEL(test_result->channel))
	{
		/*form header*/
		packet_frame_t packet_tx;
		packet_tx.header.frame_id = TEST_JIG_TO_HOST;
		packet_tx.header.device_id = device;
		packet_tx.header.cmd = JIG_CMD_PRINT_TEST_RESULT;
		packet_tx.header.payload_len = sizeof(test_result);
		memcpy((uint8_t*)&packet_tx.payload, (uint8_t*)test_result, packet_tx.header.payload_len);
		return host_tx_comm_fsm_write_packet(handle, &packet_tx);
	}
	return 0;
}

/**
 * @brief 
 * 
 * @param host_comm 
 * @param packet 
 * @return uint8_t 
 */

uint8_t host_tx_comm_fsm_write_dbg_msg(host_tx_comm_fsm_t *handle, char *dbg_msg)
{
	/* Check frame identifier */
	if (IS_VALID_DEVICE_ID(device) && dbg_msg != NULL)
	{
		/*form header*/
		packet_frame_t packet_tx;
		packet_tx.header.frame_id = TEST_JIG_TO_HOST;
		packet_tx.header.device_id = device;
		packet_tx.header.cmd = JIG_CMD_PRINT_DBG_MESSAGE;
		packet_tx.header.payload_len = strlen(dbg_msg);

		/*copy dbg message to payload*/
        if(packet_tx.header.payload_len > 0 && packet_tx.header.payload_len < PAYLOAD_BUFF_SIZE)
        {
		    memcpy((uint8_t*)&packet_tx.payload, dbg_msg, packet_tx.header.payload_len);
        }

		/*Write Data*/
        return host_tx_comm_fsm_write_packet(handle, &packet_tx);
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
    if(IS_HOST_TX_COMM_EVENT_EXT(event))
    {
    	handle->event.external = event;
    }

}

static uint8_t get_crc(uint8_t *packet, uint8_t len)
{
	uint8_t crc = packet[0];
	for (uint8_t counter = 1; counter < len; counter++) {
		crc ^= packet[counter];
	}
	return crc;
}
