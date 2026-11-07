#include "main.h"
#include "board.h"
#include <stdio.h>
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  Board_Init_GPIO();
  Board_Init_USART1();

  uint8_t state = 0;
  uint8_t key_last = HAL_GPIO_ReadPin(KEY_PORT,KEY_PIN);
  uint8_t shake_wait =0;
  uint32_t tick_heart = 0;
  uint32_t tick_key = 0;
  
  
  tick_heart = HAL_GetTick();
  while (1)
  {
    
    uint8_t key_now = HAL_GPIO_ReadPin(KEY_PORT,KEY_PIN);
    if(HAL_GetTick() - tick_heart >= HEARTBEAT_MS)
    {
      tick_heart = HAL_GetTick();
      char buf[64];
      int len = snprintf(buf,sizeof(buf),"心跳时间：%lu\r\n",HAL_GetTick());
      HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);

    }
    if( key_now != key_last && shake_wait ==0)
    {
      tick_key = HAL_GetTick();
      shake_wait = 1;
    }
      if(shake_wait ==1 && (HAL_GetTick() - tick_key >= 20))
      {
        shake_wait = 0;
        if( key_now != key_last )
        {
          key_last = key_now;
          if(key_now == 0)
          {
            if( state ==1 )
            {
              HAL_GPIO_WritePin(LED_PORT,LED_PIN,GPIO_PIN_RESET);
              
            }
            else
            {
              HAL_GPIO_WritePin(LED_PORT,LED_PIN,GPIO_PIN_SET);
              
            }
            state = !state;
            char buf[128];
            int len = snprintf(buf,sizeof(buf),"LED状态：%d\r\n",state);
            HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, HAL_MAX_DELAY);
          }

        }

      }
    
  }
}