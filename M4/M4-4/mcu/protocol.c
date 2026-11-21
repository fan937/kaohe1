#include "protocol.h"
#include <stdio.h>
#include <string.h>
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;
ProtocolParse_t parser;
extern volatile uint32_t g_last_cmd_tick;
extern volatile uint8_t  g_host_online;
volatile uint16_t bad_frame_cnt = 0;
volatile uint16_t timeout_cnt = 0;
volatile uint16_t overflow_cnt = 0;


int16_t target_left_rpm = 0;
int16_t target_right_rpm = 0;

void Parser_Reset(void)
{
    parser.state = WAIT_HEAD1;
    parser.len = 0;
    parser.msg_id = 0;
    parser.payload_idx = 0;
    parser.crc_recv = 0;
    parser.last_tick = HAL_GetTick();
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

void Protocol_ParseByte(uint8_t ch)
{
    uint32_t now = HAL_GetTick();

    // ==========修复：只有不在空闲等待帧头，才做帧内字节间隔超时==========
    
    parser.last_tick = now;

    switch(parser.state)
    {
        case WAIT_HEAD1:
            if(ch == FRAME_HEAD1)
            {   
                
                parser.state = WAIT_HEAD2;
            }
            else
            {
                
                parser.state = WAIT_HEAD1; 
            }
            break;

        case WAIT_HEAD2:
            if(ch == FRAME_HEAD2)
            {
                parser.state = WAIT_LEN;
            }
            else if(ch == FRAME_HEAD1)
            {
                parser.state = WAIT_HEAD2;   /* AA AA 55：这个 AA 就是新帧头，留在这里别丢 */
            }
            else
            {   
                parser.state = WAIT_HEAD1;
            }
            break;

        case WAIT_LEN:
            parser.len = ch;
            if(parser.len > MAX_PAYLOAD_LEN)
            {   
                
                bad_frame_cnt++;
                Parser_Reset();
                break;
            }
            parser.state = WAIT_DATA;
            parser.payload_idx = 0;
            break;

        case WAIT_DATA:
            if(parser.payload_idx == 0)
            {
                parser.msg_id = ch;
            }
            else
            {
                parser.payload[parser.payload_idx - 1] = ch;
            }
            parser.payload_idx++;
            if(parser.payload_idx >= (parser.len + 1))
            {
                parser.state = WAIT_CRC_LOW;
            }
            break;

        case WAIT_CRC_LOW:
            parser.crc_recv = ch;
            parser.state = WAIT_CRC_HIGH;
            break;

        case WAIT_CRC_HIGH:
            parser.crc_recv |= ((uint16_t)ch << 8);
            {
                uint8_t check_buf[2 + 1 + 1 + MAX_PAYLOAD_LEN];
                uint16_t check_len = 0;
                check_buf[check_len++] = FRAME_HEAD1;
                check_buf[check_len++] = FRAME_HEAD2;
                check_buf[check_len++] = parser.len;
                check_buf[check_len++] = parser.msg_id;
                for(int i = 0; i < parser.len; i++)
                {
                    check_buf[check_len++] = parser.payload[i];
                }
                uint16_t crc_calc = CRC16_Modbus(check_buf, check_len);

                if(crc_calc != parser.crc_recv)
                {
                    /* CRC 校验失败 → 坏帧 */
                    bad_frame_cnt++;
                }
                else if(parser.msg_id == 0x01 && parser.len == 4)
                {
                    /* 速度指令 小端 */
                    target_left_rpm  = (int16_t)((parser.payload[1] << 8) | parser.payload[0]);
                    target_right_rpm = (int16_t)((parser.payload[3] << 8) | parser.payload[2]);

                    /* ===== 场景③：上位机在线打戳 ===== */
                    g_last_cmd_tick = HAL_GetTick();
                    g_host_online   = 1;
                }
                else
                {
                    /* CRC 对但 msg_id / len 不合法 → 也算坏帧（语义白名单） */
                    bad_frame_cnt++;
                }
            }
            Parser_Reset();
            break;

        default:
            Parser_Reset();
            break;
    }
}


void Protocol_MakeStatusFrame(uint8_t *tx_buf,
                              int16_t l_rpm,
                              int16_t r_rpm,
                              int16_t pitch,
                              int16_t heartbeat,
                              uint16_t bad_frame,
                              uint16_t timeout,
                              uint16_t overflow)
{
    uint8_t idx = 0;
    tx_buf[idx++] = FRAME_HEAD1;
    tx_buf[idx++] = FRAME_HEAD2;
    tx_buf[idx++] = 14;     // payload改成14字节！
    tx_buf[idx++] = 0x02;   // msg_id

    //小端打包
    tx_buf[idx++] = l_rpm & 0xFF;
    tx_buf[idx++] = (l_rpm >> 8) & 0xFF;

    tx_buf[idx++] = r_rpm & 0xFF;
    tx_buf[idx++] = (r_rpm >> 8) & 0xFF;

    tx_buf[idx++] = pitch & 0xFF;
    tx_buf[idx++] = (pitch >> 8) & 0xFF;

    tx_buf[idx++] = heartbeat & 0xFF;
    tx_buf[idx++] = (heartbeat >> 8) & 0xFF;

    tx_buf[idx++] = bad_frame & 0xFF;
    tx_buf[idx++] = (bad_frame >> 8) & 0xFF;

    tx_buf[idx++] = timeout & 0xFF;
    tx_buf[idx++] = (timeout >> 8) & 0xFF;

    tx_buf[idx++] = overflow & 0xFF;
    tx_buf[idx++] = (overflow >> 8) & 0xFF;

    uint16_t crc = CRC16_Modbus(tx_buf, 2+1+1+14);
    tx_buf[idx++] = crc & 0xFF;
    tx_buf[idx++] = (crc >> 8) & 0xFF;
}