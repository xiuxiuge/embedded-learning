#include "ring_buffer.h"

void ring_buffer_init(ring_buffer_t *rb)
{
    rb->write_index = 0;
    rb->read_index = 0;
    rb->count = 0;
}

bool ring_buffer_put(ring_buffer_t *rb, uint8_t data)
{
    if (ring_buffer_is_full(rb))
        return false;  // 满了就丢弃（可以改成覆盖模式，但先保守）

    rb->buffer[rb->write_index] = data;
    rb->write_index = (rb->write_index + 1) % RB_SIZE;
    rb->count++;
    return true;
}

bool ring_buffer_get(ring_buffer_t *rb, uint8_t *data)
{
    if (ring_buffer_is_empty(rb))
        return false;

    *data = rb->buffer[rb->read_index];
    rb->read_index = (rb->read_index + 1) % RB_SIZE;
    rb->count--;
    return true;
}

bool ring_buffer_is_empty(ring_buffer_t *rb)
{
    return (rb->count == 0);
}

bool ring_buffer_is_full(ring_buffer_t *rb)
{
    return (rb->count == RB_SIZE);
}