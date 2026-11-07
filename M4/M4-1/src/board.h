#ifndef __BOARD_H
#define __BOARD_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "board_config.h"

#if LED_ACTIVE_HIGH
#define LED_ON()  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET)
#define LED_OFF() HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET)
#else
#define LED_ON()  HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET)
#define LED_OFF() HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET)
#endif

#if KEY_ACTIVE_LOW
#define KEY_ON()  (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_RESET)
#define KEY_OFF() (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_SET)
#else
#define KEY_ON()  (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_SET)
#define KEY_OFF() (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_SET)
#endif

void Board_Init_GPIO(void);
void Board_Init_USART1(void);
void SystemClock_Config(void);
void Error_Handler(void);
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#endif 
extern UART_HandleTypeDef huart1;
#ifdef __cplusplus
}
#endif

#endif