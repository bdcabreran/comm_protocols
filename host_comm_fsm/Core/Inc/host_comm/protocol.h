/**
 * @file protocol.h
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief Protocol Scheme for Host-Target communication
 * @version 0.1
 * @date 2021-08-17
 */
#include "stdint.h"

typedef struct
{
    typedef union
    {
        uint8_t cmd;
        uint8_t res;
        uint8_t evt;
    }type;

    uint8_t  dest_id;
    uint8_t  src_id;
    uint8_t  seq_num;
    uint16_t payload_len;

}packet_header_t;

typedef struct
{

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


/* Host Header Types */
typedef enum 
{
    HOST_TO_TARGET_CMD_START = 0x00,
    HOST_TO_TARGET_CMD_END = 0x55
}host_to_target_cmd_t;

typedef enum
{
    HOST_TO_TARGET_EVT_START = 0x56,
    HOST_TO_TARGET_EVT_END = 0xAB
}host_to_target_evt_t;

typedef enum
{
    HOST_TO_TARGET_RES_START = 0xAC,
    HOST_TO_TARGET_RES_END = 0xFF
}host_to_target_resp_t


/* Target Header Types*/
typedef enum 
{
    TARGET_TO_HOST_CMD_START = 0x00,
    TARGET_TO_HOST_CMD_END = 0x55
}target_to_host_cmd_t;

typedef enum
{
    TARGET_TO_HOST_EVT_START = 0x56,
    TARGET_TO_HOST_EVT_END = 0xAB
}target_to_host_evt_t;

typedef enum
{
    TARGET_TO_HOST_RES_START = 0xAC,
    TARGET_TO_HOST_RES_END = 0xFF
}target_to_host_resp_t



