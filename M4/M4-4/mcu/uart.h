#ifndef __UART_H
#define __UART_H
#include "stm32f1xx_hal.h"
#include "ringbuf.h"

extern UART_HandleTypeDef huart1;
extern RingBuf_t uart_rx_rb;

void UART_InitIT(void);
void UART_SendBytes(uint8_t *data, uint16_t len);

#endif