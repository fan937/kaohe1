#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define FRAME_HEAD1     0xAA
#define FRAME_HEAD2     0x55
#define MAX_PAYLOAD_LEN 32
#define FRAME_TIMEOUT_MS 50

typedef enum
{
    WAIT_HEAD1,
    WAIT_HEAD2,
    WAIT_LEN,
    WAIT_DATA,
    WAIT_CRC_LOW,
    WAIT_CRC_HIGH
} ProtState_t;

typedef struct
{
    ProtState_t state;
    uint8_t len;
    uint8_t msg_id;
    uint8_t payload_idx;
    uint16_t crc_recv;
    uint32_t last_tick;
    uint8_t payload[MAX_PAYLOAD_LEN];
} ProtocolParse_t;

typedef struct
{
    int16_t l_rpm;
    int16_t r_rpm;
    int16_t pitch;
    uint16_t heartbeat;
    uint16_t bad_frame;
    uint16_t timeout_cnt;
    uint16_t overflow_cnt;
    int valid;
} StatusFrame_t;

void Parser_Reset(ProtocolParse_t *p);
uint16_t CRC16_Modbus(uint8_t *buf, uint16_t len);
void Protocol_ParseByte(ProtocolParse_t *p, uint8_t ch, StatusFrame_t *out_status);
int Protocol_MakeSpeedFrame(uint8_t *tx_buf, int16_t left_rpm, int16_t right_rpm);
void     Protocol_CountBad(void);
uint32_t Protocol_GetBad(void);
uint32_t Protocol_GetOk(void);

uint64_t get_ms(void);

#endif