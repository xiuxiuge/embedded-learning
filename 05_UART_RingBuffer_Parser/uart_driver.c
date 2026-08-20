/*
 * uart_driver.c
 *
 *  Created on: 2026年8月20日
 *      Author: 15332
 */


#include "uart_driver.h"

extern UART_HandleTypeDef huart2;

uint8_t uart_rx_byte;           // 中断接收临时存放
ring_buffer_t uart_rb;          // 全局环形缓冲区

void UART_RX_Start(void)
{
    HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1);
}

// 中断回调（HAL 弱函数重写）
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // 收到一个字节，放入环形缓冲区
        ring_buffer_put(&uart_rb, uart_rx_byte);

        // 翻转LD2提示已接收
        HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);

        // 重新开启下一次中断接收
        UART_RX_Start();
    }
}

// 暂时空实现，以后扩展
void UART_Process(void)
{
    // 预留
}
