/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "init_peripherals.h"
#include "stdio.h"

#define HEARTBEAT_PERIOD_MS (200)
void heartbeat_handler(void);


int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

void print_startup_message(void)
{
	printf("**************************************\r\n");
	printf("Brief:\t Communication Protocol FSM\r\n");
	printf("Author:\t Bayron Cabrera \r\n");
	printf("Board:\t Nucleo F411RE \r\n");
	printf("Date:\t %s\r\n", __DATE__);
	printf("**************************************\r\n");
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU configuration */
  peripherals_init();
  print_startup_message();

  /* Infinite loop */
  while (1)
  {

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

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART2)
  {
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
