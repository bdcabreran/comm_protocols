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
#include "uart_driver.h"

extern void Error_Handler(void);

/* Handle driver type used for host communication*/
UART_HandleTypeDef huart2;

typedef struct
{
    struct
    {
        uint8_t buffer[RX_DATA_BUFF_SIZE]; /* Received Data over UART are stored in this buffer */
        c_buff_handle_t c_buff;            /* pointer typedef to circular buffer struct */
        uint8_t byte;                      /* used to active RX reception interrupt mode */ 
    } rx;

    struct
    {
        uint8_t buffer[TX_DATA_BUFF_SIZE]; /* Data to be transmitted via UART are stored in this buffer */
        c_buff_handle_t c_buff;           /* pointer typedef to circular buffer struct */
    } tx;


}comm_driver_data_t;

/**/
static comm_driver_data_t driver_data; 

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
uint8_t uart_init(void)
{
    /*Init Uart device*/
    MX_USART2_UART_Init();

    /*Init Circular Buffer*/
    driver_data.tx.c_buff = circular_buff_init(driver_data.tx.buffer, TX_DATA_BUFF_SIZE);
    driver_data.rx.c_buff = circular_buff_init(driver_data.rx.buffer, RX_DATA_BUFF_SIZE);

    /*Start Reception of data*/
    HAL_UART_Receive_IT(&huart2, &driver_data.rx.byte, 1);

    printf("comm driver info : uart2 initialized\r\n");

    return 1;
}

uint8_t uart_get_rx_data_len(void)
{
    return circular_buff_get_data_len(driver_data.rx.c_buff);
}


uint8_t uart_read_rx_data(uint8_t *data, uint8_t len)
{
    return circular_buff_read(driver_data.rx.c_buff, data, len);
}


uint8_t uart_fetch_rx_data(uint8_t *data, uint8_t len)
{
    return circular_buff_fetch(driver_data.rx.c_buff, data, len);
}


uint8_t uart_clear_rx_data(void)
{
    circular_buff_reset(driver_data.rx.c_buff);
    return 1;
}

uint8_t uart_transmit(uint8_t *data, uint8_t len)
{
    return HAL_UART_Transmit(&huart2, data, len, HAL_MAX_DELAY);
}

uint8_t uart_transmit_it(uint8_t *data, uint8_t len)
{
    /* Write data to circular buffer */
    if (circular_buff_write(driver_data.tx.c_buff, data, len) == CIRCULAR_BUFF_OK)
    {
        if (huart2.gState == HAL_UART_STATE_READY)
        {
            uint8_t byte;
            circular_buff_get(driver_data.tx.c_buff, &byte);
            HAL_UART_Transmit_IT(&huart2, &byte, 1);
            return 1;
        }
        else
        {
            printf("comm driver warning:\t uart busy\r\n");
            return 0;
        }
    }

    printf("comm driver error:\t circular buffer cannot write request\r\n");
    return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  { 
    /*check for pendings transfers */
    static uint8_t data_chunk[MAX_DATA_CHUNK_SIZE];
    uint16_t data_len = circular_buff_get_data_len(driver_data.tx.c_buff);

    if(data_len)
    {
        data_len = (data_len >= MAX_DATA_CHUNK_SIZE ) ? (MAX_DATA_CHUNK_SIZE - 1) : data_len;
        circular_buff_read(driver_data.tx.c_buff, data_chunk, data_len);
        HAL_UART_Transmit_IT(&huart2, data_chunk, data_len);
    }

    printf("comm driver info:\t irq uart tx complete\r\n");
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        /*Set Uart Data reception for next byte*/
        HAL_UART_Receive_IT(&huart2, &driver_data.rx.byte, 1);

        if(circular_buff_write(driver_data.rx.c_buff, &driver_data.rx.byte, 1) !=  CIRCULAR_BUFF_OK)
        {
            /*Reinit ring buffer*/
            circular_buff_reset(driver_data.rx.c_buff);
        }
    }
}






