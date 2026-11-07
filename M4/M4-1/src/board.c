#include "board.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
UART_HandleTypeDef huart1;
void Board_Init_USART1(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
    const uint8_t test_msg[] = "USART OK\r\n";
    HAL_UART_Transmit(&huart1, test_msg, sizeof(test_msg) - 1, 0xFFFF);
 
}

int fputc(int ch, FILE *f)
{   
    uint8_t byte = (uint8_t)ch;
    HAL_UART_Transmit(&huart1,&byte , 1 , HAL_MAX_DELAY);
    return ch;
}

static void ENABLE_GPIO_CLOCK(GPIO_TypeDef *port)
{
    if(port == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if(port == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if(port == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }


}
void Board_Init_GPIO(void)
{
    GPIO_InitTypeDef gpio_conf ={0};
    ENABLE_GPIO_CLOCK(LED_PORT);
    gpio_conf.Pin = LED_PIN;
    gpio_conf.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_conf.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT,&gpio_conf);
    ENABLE_GPIO_CLOCK(KEY_PORT);
    gpio_conf.Pin = KEY_PIN;
    gpio_conf.Mode = GPIO_MODE_INPUT;
    gpio_conf.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY_PORT,&gpio_conf);
}
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void) file;
    (void) line;
    __disable_irq();
    while (1)
    {
    }
}
#endif 