#include "protocol.h"
#include <string.h>
#include <stdio.h>

static uint32_t s_bad_local = 0;
static uint16_t s_junk_cnt  = 0;
static uint32_t s_frame_ok  = 0;
#define JUNK_LIMIT 8

void     Protocol_CountBad(void) { s_bad_local++; }
uint32_t Protocol_GetBad(void)   { return s_bad_local; }
uint32_t Protocol_GetOk(void)    { return s_frame_ok; }

void Parser_Reset(ProtocolParse_t *p)
{
    p->state = WAIT_HEAD1;
    p->len = 0;
    p->msg_id = 0;
    p->payload_idx = 0;
    p->crc_recv = 0;
    p->last_tick = 0;
}

uint16_t CRC16_Modbus(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for(uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void Protocol_ParseByte(ProtocolParse_t *p, uint8_t ch, StatusFrame_t *out_status)
{
    memset(out_status, 0, sizeof(StatusFrame_t));
    p->last_tick = (uint32_t)get_ms();

    switch(p->state)
    {
        case WAIT_HEAD1:
    if(ch == FRAME_HEAD1)
    {
        s_junk_cnt = 0;                 /* 找到帧头，垃圾段清零 */
        p->state = WAIT_HEAD2;
    }
    else
    {
        s_junk_cnt++;                   /* 数垃圾字节 */
        if(s_junk_cnt >= JUNK_LIMIT)
        {
            s_junk_cnt = 0;
            s_bad_local++;              /* 每32个垃圾字节 = 一次坏帧 */
        }
    }
    break;
        case WAIT_HEAD2:
    if(ch == FRAME_HEAD2)
    {
        p->state = WAIT_LEN;
    }
    else if(ch == FRAME_HEAD1)
    {
        p->state = WAIT_HEAD2;          /* AA AA 55：第二个 AA 就是新帧头，留着别丢，也不计坏帧 */
    }
    else
    {
        s_bad_local++;                  /* 假帧头：0xAA 后面不是 0x55 */
        p->state = WAIT_HEAD1;
    }
    break;
        case WAIT_LEN:
    p->len = ch;
    if(p->len == 0 || p->len > MAX_PAYLOAD_LEN)
    {
        s_bad_local++;                  /* 长度非法 */
        Parser_Reset(p);
        break;
    }
    p->state = WAIT_DATA;
    p->payload_idx = 0;
    break;
        case WAIT_DATA:
    if(p->payload_idx == 0)
    {
        p->msg_id = ch;
    }
    else if((p->payload_idx - 1) < MAX_PAYLOAD_LEN)
    {
        p->payload[p->payload_idx - 1] = ch;
    }
    p->payload_idx++;
    if(p->payload_idx >= (p->len + 1))
    {
        p->state = WAIT_CRC_LOW;
    }
    break;
        case WAIT_CRC_LOW:
            p->crc_recv = ch;
            p->state = WAIT_CRC_HIGH;
            break;
        case WAIT_CRC_HIGH:
            p->crc_recv |= ((uint16_t)ch << 8);
                        {
                uint8_t  check_buf[2 + 1 + 1 + MAX_PAYLOAD_LEN];
                uint16_t check_len = 0;
                check_buf[check_len++] = FRAME_HEAD1;
                check_buf[check_len++] = FRAME_HEAD2;
                check_buf[check_len++] = p->len;
                check_buf[check_len++] = p->msg_id;
                for(int i = 0; i < p->len; i++)
                {
                    check_buf[check_len++] = p->payload[i];
                }
                uint16_t crc_calc = CRC16_Modbus(check_buf, check_len);

                /* ===== 三道校验：CRC → 语义 → 取值范围 ===== */
                int16_t l_chk = 0, r_chk = 0, q_chk = 0;
                int ok_sem = 0;

                if(crc_calc == p->crc_recv)                    /* 第 1 道：CRC */
                {
                    if(p->msg_id == 0x02 && p->len == 14)      /* 第 2 道：语义 */
                    {
                        l_chk = (int16_t)((p->payload[1] << 8) | p->payload[0]);
                        r_chk = (int16_t)((p->payload[3] << 8) | p->payload[2]);
                        q_chk = (int16_t)((p->payload[5] << 8) | p->payload[4]);

                        if(l_chk >= -300 && l_chk <= 300 &&    /* 第 3 道：范围 */
                           r_chk >= -300 && r_chk <= 300 &&
                           q_chk >= -9000 && q_chk <= 9000)
                        {
                            ok_sem = 1;
                        }
                    }
                }

                if(ok_sem)
                {
                    s_frame_ok++;
                    out_status->l_rpm        = l_chk;
                    out_status->r_rpm        = r_chk;
                    out_status->pitch        = q_chk;
                    out_status->heartbeat    = (uint16_t)((p->payload[7] << 8) | p->payload[6]);
                    out_status->bad_frame    = (uint16_t)((p->payload[9] << 8) | p->payload[8]);
                    out_status->timeout_cnt  = (uint16_t)((p->payload[11] << 8) | p->payload[10]);
                    out_status->overflow_cnt = (uint16_t)((p->payload[13] << 8) | p->payload[12]);
                    out_status->valid = 1;
                }
                else
                {
                    s_bad_local++;
                }
            }
            Parser_Reset(p);
            break;
    }
}

int Protocol_MakeSpeedFrame(uint8_t *tx_buf, int16_t left_rpm, int16_t right_rpm)
{
    uint8_t idx = 0;
    tx_buf[idx++] = FRAME_HEAD1;
    tx_buf[idx++] = FRAME_HEAD2;
    tx_buf[idx++] = 0x04;
    tx_buf[idx++] = 0x01;

    tx_buf[idx++] = left_rpm & 0xFF;
    tx_buf[idx++] = (left_rpm >> 8) & 0xFF;
    tx_buf[idx++] = right_rpm & 0xFF;
    tx_buf[idx++] = (right_rpm >> 8) & 0xFF;

    uint16_t crc = CRC16_Modbus(tx_buf, idx);
    tx_buf[idx++] = crc & 0xFF;
    tx_buf[idx++] = (crc >> 8) & 0xFF;
    return idx;
}