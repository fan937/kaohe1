#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

#define FRAME_HEAD1     0xAA
#define FRAME_HEAD2     0x55
#define FRAME_TIMEOUT_MS 50
#define MAX_PAYLOAD_LEN 20

typedef enum
{
    WAIT_HEAD1,
    WAIT_HEAD2,
    WAIT_LEN,
    WAIT_DATA,
    WAIT_CRC_LOW,
    WAIT_CRC_HIGH,
}ProtocolState_t;

typedef struct
{
    ProtocolState_t state;
    uint8_t len;
    uint8_t msg_id;
    uint8_t payload_idx;
    uint8_t payload[MAX_PAYLOAD_LEN];
    uint16_t crc_recv;
    uint32_t last_tick;
}ProtocolParse_t;

extern ProtocolParse_t parser;
extern volatile uint16_t bad_frame_cnt;
extern volatile uint16_t timeout_cnt;
extern volatile uint16_t overflow_cnt;

extern int16_t target_left_rpm;
extern int16_t target_right_rpm;
extern int16_t pitch;

void Parser_Reset(void);
uint16_t CRC16_Modbus(uint8_t *buf, uint16_t len);
void Protocol_ParseByte(uint8_t ch);

void Protocol_MakeStatusFrame(uint8_t *tx_buf,
                              int16_t l_rpm,
                              int16_t r_rpm,
                              int16_t pitch,
                              int16_t heartbeat,
                              uint16_t bad_frame,
                              uint16_t timeout,
                              uint16_t overflow);
#endif