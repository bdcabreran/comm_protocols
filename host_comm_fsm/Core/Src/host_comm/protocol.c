#include "protocol.h"

void print_buff_hex(uint8_t *buff, size_t len)
{
    printf("buffer hex format : [ ");
    for (size_t i = 0; i < len; i++)
    {
        printf(" 0x%.2X", buff[i]);
    }
    printf(" ]\r\n");
}

void print_buff_ascii(uint8_t *buff, size_t len)
{
    printf("buffer ascii format : [ ");
    for (size_t i = 0; i < len; i++)
    {
        printf(" %c", buff[i]);
    }
    printf("\r\n");
}

