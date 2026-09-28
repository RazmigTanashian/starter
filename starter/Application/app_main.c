#include "app_main.h"

void mainTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
	  // Acts as a visual heart beat when the application begins
	  HAL_GPIO_TogglePin(USER_LED_RED_GPIO_Port, USER_LED_RED_Pin);
	  HAL_Delay(750);
  }
  /* USER CODE END 5 */
}
