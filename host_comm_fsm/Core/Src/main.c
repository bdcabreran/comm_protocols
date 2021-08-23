/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "peripherals_init.h"
#include "host_comm_tx_fsm.h"
#include "host_comm_rx_fsm.h"
#include "stdio.h"
#include "tdd.h"

#include "uart_driver.h"

#define HEARTBEAT_PERIOD_MS (200)
void heartbeat_handler(void);


void print_startup_message(void)
{
	printf("**************************************\r\n");
	printf("Brief:\t Communication Protocol FSM\r\n");
	printf("Author:\t Bayron Cabrera \r\n");
	printf("Board:\t Nucleo F411RE \r\n");
	printf("Date:\t %s\r\n", __DATE__);
	printf("**************************************\r\n");
}

uint8_t rx_buff[1024];
uint8_t tx_buff[512];

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU configuration */
  peripherals_init();
  print_startup_message();

  /* init host tx fsm*/
  host_comm_tx_fsm_init(&host_comm_tx_handle);
  host_comm_rx_fsm_init(&host_comm_rx_handle);

  /* run tdd #0*/
  tx_comm_test_0();

  /* Infinite loop */
  while (1)
  {
    host_comm_rx_fsm_run(&host_comm_rx_handle);
    host_comm_tx_fsm_run(&host_comm_tx_handle);

    heartbeat_handler();
  }
}

void heartbeat_handler(void)
{
  static uint32_t last_tick = 0;
  if (HAL_GetTick() - last_tick > HEARTBEAT_PERIOD_MS)
  {
    last_tick = HAL_GetTick();
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
