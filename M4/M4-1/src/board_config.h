#ifndef __BOARD_CONFIG_H
#define __BOARD_CONFIG_H

#include "stm32f1xx_hal.h"

#define LED_PORT        GPIOB
#define LED_PIN         GPIO_PIN_0
#define LED_ACTIVE_HIGH 0

#define KEY_PORT        GPIOA
#define KEY_PIN         GPIO_PIN_0
#define KEY_ACTIVE_LOW  1

#define UART_BAUD       115200
#define HEARTBEAT_MS    500


#endif
