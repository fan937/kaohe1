#include "uart.h"

extern UART_HandleTypeDef huart1;
RingBuf_t uart_rx_rb;
uint8_t rx_byte;

void UART_InitIT(void)
{
    RingBuf_Init(&uart_rx_rb);
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void UART_SendBytes(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, data, len, 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        uint8_t ch = rx_byte;
        if(RingBuf_Push(&uart_rx_rb, ch) == 0)
        {
            extern volatile uint32_t overflow_cnt;
            overflow_cnt++;
        }
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}