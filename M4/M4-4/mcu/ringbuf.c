#include "ringbuf.h"
// 引入外部溢出计数器，定义在protocol.c
extern volatile uint16_t overflow_cnt;

void RingBuf_Init(RingBuf_t *rb)
{
    rb->wr = 0;
    rb->rd = 0;
}

uint8_t RingBuf_Push(RingBuf_t *rb, uint8_t dat)
{
    uint16_t next_wr = (rb->wr + 1) % RING_BUF_LEN;
    if(next_wr == rb->rd)
    {
        // 缓冲区已满，丢弃新来字节，溢出计数+1
        overflow_cnt++;
        return 0; //入队失败
    }
    rb->buf[rb->wr] = dat;
    rb->wr = next_wr;
    return 1;
}

uint8_t RingBuf_Pop(RingBuf_t *rb, uint8_t *dat)
{
    if(rb->rd == rb->wr)
        return 0;
    *dat = rb->buf[rb->rd];
    rb->rd = (rb->rd + 1) % RING_BUF_LEN;
    return 1;
}

uint8_t RingBuf_IsEmpty(RingBuf_t *rb)
{
    return (rb->rd == rb->wr);
}

uint8_t RingBuf_IsFull(RingBuf_t *rb)
{
    return ((rb->wr + 1) % RING_BUF_LEN == rb->rd);
}