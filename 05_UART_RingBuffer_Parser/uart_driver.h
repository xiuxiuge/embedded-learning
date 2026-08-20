#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "main.h"
#include "ring_buffer.h"

// 外部可访问的环形缓冲区（供主循环读取）
extern ring_buffer_t uart_rb;

void UART_RX_Start(void);  // 启动 UART 中断接收
void UART_Process(void);   // 将来扩展用，暂时可空

#endif