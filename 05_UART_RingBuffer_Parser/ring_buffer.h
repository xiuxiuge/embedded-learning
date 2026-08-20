#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RB_SIZE 128  // 环形缓冲区大小

typedef struct
{
    uint8_t buffer[RB_SIZE]; // 环形缓冲区存储数据
    uint16_t write_index;    // 写索引
    uint16_t read_index;     // 读索引
    uint16_t count;          // 当前已存数据个数（方便判断空/满）
} ring_buffer_t;

// 对外接口
void ring_buffer_init(ring_buffer_t *rb);                // 初始化环形缓冲区
bool ring_buffer_put(ring_buffer_t *rb, uint8_t data);   // 写入数据到环形缓冲区
bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data);  // 从环形缓冲区读取数据
bool ring_buffer_is_empty(ring_buffer_t *rb);            // 判断环形缓冲区是否为空
bool ring_buffer_is_full(ring_buffer_t *rb);             // 判断环形缓冲区是否已满

#endif