
#include "time_event.h"
#include "stm32f4xx_hal.h"
#include "host_comm_tx_fsm.h"

/**
 * @brief Systick Callback Function 
 * @note  This callback is executed every ms
 */
void HAL_SYSTICK_Callback(void)
{
    /* update FSM time events*/
    host_tx_comm_fsm_time_event_update(&host_tx_comm_handle);

}
