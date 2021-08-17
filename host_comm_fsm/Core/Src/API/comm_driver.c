/**
 * @file comm_driver.c
 * @author Bayron Cabrera (bayron.cabrera@titoma.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "comm_driver.h"

/* Handle driver type used for host communication*/
UART_HandleTypeDef huart2;

/* Data buffer Control */
static c_buff_handle_t cb_tx;
static c_buff_handle_t cb_rx;


/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}


/**
 * @brief Init host comm peripheral interface
 * 
 * @param rx_buff buffer in stack reserved for data reception 
 * @param tx_buff buffer in stack reserved for data transmission
 * @return uint8_t 
 */
uint8_t host_comm_init(uint8_t *rx_buff, uint16_t rx_buff_len,
                       uint8_t *tx_buff, uint16_t tx_buff_len)
{
    /*Init Uart device*/
    MX_USART2_UART_Init();

    /*Init Circular Buffer*/
    cb_tx = circular_buff_init(tx_buff, tx_buff_len);
    cb_rx = circular_buff_init(rx_buff, rx_buff_len);

    printf("comm driver info : uart2 initialized\r\n");
}

uint8_t host_comm_get_rx_data_len(void)
{

}


uint8_t host_comm_read_rx_data(uint8_t *data, uint8_t len)
{

}


uint8_t host_comm_fetch_rx_data(uint8_t *data, uint8_t len)
{

}


uint8_t host_comm_clear_rx_data(void)
{

}


uint8_t host_comm_transmit(uint8_t *data, uint8_t len)
{

}

uint8_t host_comm_transmit_it(uint8_t *data, uint8_t len)
{
    /* Write data to circular buffer */
    if (circular_buff_write(cb_tx, data, len) == CIRCULAR_BUFF_OK)
    {
        if (huart2.gState == HAL_UART_STATE_READY)
        {
            uint8_t byte;
            circular_buff_get(cb_tx, &byte);
            HAL_UART_Transmit_IT(&huart2, &byte, 1);
            return 1;
        }
        else
        {
            printf("warning:\t uart busy\r\n");
            return 0;
        }
    }

    printf("error:\t uart transmission error\r\n");
    return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  { 
    /*check for pendings transfers */
    static uint8_t data_chunk[MAX_DATA_CHUNK_SIZE];
    uint16_t data_len = circular_buff_get_data_len(cb_tx);

    if(data_len)
    {
        data_len = (data_len >= MAX_DATA_CHUNK_SIZE ) ? (MAX_DATA_CHUNK_SIZE - 1) : data_len;
        circular_buff_read(cb_tx, data_chunk, data_len);
        HAL_UART_Transmit_IT(&huart2, data_chunk, data_len);
    }

    printf("info :\t irq uart tx complete\r\n");
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {

    }

}






