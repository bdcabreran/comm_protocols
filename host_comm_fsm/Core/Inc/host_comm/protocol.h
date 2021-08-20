/**
 * @file protocol.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief Protocol Scheme for Host-Target communication
 * @version 0.1
 * @date 2021-08-17
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "stdint.h"
#include "stdio.h"

#define MAX_PAYLOAD_SIZE	(256)

/* 1 byte = 256 possible cmd/res/evt */
#define CMD_START   (0x00)
#define CMD_END     (0x55)
#define EVT_START   (0x56)
#define EVT_END     (0xAB)
#define RES_START   (0xAC)
#define RES_END     (0xFF)

/* Host / Target IDs */
#define TARGET_TO_HOST_DIR   (0xAA)
#define HOST_TO_TARGET_DIR   (0xBB)

/* Packet Format Sizes */
#define PREAMBLE_SIZE_BYTES     sizeof(uint16_t)
#define POSTAMBLE_SIZE_BYTES    sizeof(uint16_t)
#define HEADER_SIZE_BYTES       sizeof(packet_header_t)
#define CRC_SIZE_BYTES          sizeof(uint32_t)

/* Preamble / Postamble bytes */
#define PREAMBLE              (0xFF7F)
#define POSTAMBLE             (0xDEDF)

typedef enum
{
    HOST_TO_TARGET,
    TARGET_TO_HOST,
}packet_dir_t;

typedef struct
{
    union
    {
        uint8_t cmd;
        uint8_t res;
        uint8_t evt;
    }type;

    packet_dir_t  dir;
    uint16_t payload_len;

}packet_header_t;

typedef struct
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];

}packet_payload_t;

typedef struct
{
    packet_header_t  header;
    packet_payload_t payload;

}packet_data_t;

typedef struct
{
    uint16_t preamble;
    packet_data_t data;
    uint32_t crc;
    uint16_t postamble;

}packet_frame_t;


/*##################################################################################################*/

/* Host Header Types */
typedef enum
{
    HOST_TO_TARGET_CMD_START = CMD_START,
    HOST_TO_TARGET_CMD_TURN_ON_LED,
    HOST_TO_TARGET_CMD_TURN_OFF_LED,
    HOST_TO_TARGET_CMD_GET_FW_VERSION,
    HOST_TO_TARGET_CMD_END = CMD_END
}host_to_target_cmd_t;

typedef enum
{
    HOST_TO_TARGET_EVT_START = EVT_START,
    HOST_TO_TARGET_EVT_END = EVT_END
}host_to_target_evt_t;

typedef enum
{
    HOST_TO_TARGET_RES_START = RES_START,
    HOST_TO_TARGET_RES_ACK,
    HOST_TO_TARGET_RES_NACK,
    HOST_TO_TARGET_RES_END = RES_END
}host_to_target_resp_t;

/*##################################################################################################*/

/* Target Header Types*/
typedef enum
{
    TARGET_TO_HOST_CMD_START = CMD_START,
    TARGET_TO_HOST_CMD_END = CMD_END
}target_to_host_cmd_t;

typedef enum
{
    TARGET_TO_HOST_EVT_START = EVT_START,
    TARGET_TO_HOST_EVT_HANDLER_ERROR,
    TARGET_TO_HOST_EVT_PRINT_DBG_MSG,
    TARGET_TO_HOST_EVT_END = EVT_END
}target_to_host_evt_t;

typedef enum
{
    TARGET_TO_HOST_RES_START = RES_START,
    TARGET_TO_HOST_RES_ACK,
    TARGET_TO_HOST_RES_NACK,
    TARGET_TO_HOST_RES_LED_ON,
    TARGET_TO_HOST_RES_LED_OFF,
    TARGET_TO_HOST_RES_FW_VERSION,
    TARGET_TO_HOST_RES_END = RES_END
}target_to_host_resp_t;

/*##################################################################################################*/
void print_buff_ascii(uint8_t *buff, size_t len);
void print_buff_hex(uint8_t *buff, size_t len);


#endif
