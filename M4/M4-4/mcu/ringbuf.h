#ifndef __RINGBUF_H
#define __RINGBUF_H

#include <stdint.h>
#define RING_BUF_LEN 128

typedef struct
{
    uint8_t buf[RING_BUF_LEN];
    uint16_t wr;
    uint16_t rd;
} RingBuf_t;

void RingBuf_Init(RingBuf_t *rb);
uint8_t RingBuf_Push(RingBuf_t *rb, uint8_t dat);
uint8_t RingBuf_Pop(RingBuf_t *rb, uint8_t *dat);
uint8_t RingBuf_IsEmpty(RingBuf_t *rb);
uint8_t RingBuf_IsFull(RingBuf_t *rb);

#endif