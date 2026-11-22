#define _POSIX_C_SOURCE 200809L
#define STALL_TIMEOUT_MS 150
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/select.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include "serial.h"
#include "ringbuf.h"
#include "protocol.h"

uint16_t g_bad_frame = 0;
uint16_t g_timeout_cnt = 0;
uint16_t g_overflow_cnt = 0;

typedef struct{
    char dev[128];
    int baud;
    char csv_path[128];
    int16_t send_rpm1;
    int16_t send_rpm2;
    int enable_send;
} AppCfg_t;

static AppCfg_t g_cfg;
static SerialDev_t g_ser;
static RingBuf_t g_rb;
static ProtocolParse_t g_parser;
static FILE *g_csv_fp = NULL;
static uint64_t g_t_lost        = 0;   /* 检测到断开的时刻 */
static uint64_t g_t_reopen      = 0;   /* 重新 open 成功的时刻 */
static uint64_t g_t_first_frame = 0;   /* 恢复后第一帧合法帧的时刻 */

#define SEND_PERIOD_MS 50

static void parse_args(int argc, char **argv, AppCfg_t *cfg)
{
    memset(cfg,0,sizeof(*cfg));
    cfg->baud = 115200;
    cfg->enable_send = 0;

    static struct option long_opts[] = {
        {"dev", required_argument, 0, 'd'},
        {"baud", required_argument,0,'b'},
        {"csv", required_argument,0,'c'},
        {"send-rpm", required_argument,0,'s'},
        {0,0,0,0}
    };
    int opt;
    while((opt=getopt_long(argc,argv,"",long_opts,NULL)) != -1){
        switch(opt){
            case 'd': strncpy(cfg->dev, optarg, sizeof(cfg->dev)-1); break;
            case 'b': cfg->baud = atoi(optarg); break;
            case 'c': strncpy(cfg->csv_path, optarg, sizeof(cfg->csv_path)-1); break;
            case 's':{
                int a,b;
                sscanf(optarg,"%d,%d",&a,&b);
                cfg->send_rpm1 = (int16_t)a;
                cfg->send_rpm2 = (int16_t)b;
                cfg->enable_send = 1;
                break;
            }
            default:
                fprintf(stderr,"Usage:\n"
"./serial_tool --dev /dev/ttyUSB0 --baud 115200 --csv log.csv --send-rpm 60,0\n");
                exit(1);
        }
    }
    if(strlen(cfg->dev)==0){
        fprintf(stderr,"need --dev\n");
        exit(1);
    }
}

uint64_t get_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IOLBF, 1 << 16);
    parse_args(argc, argv, &g_cfg);
    RingBuf_Init(&g_rb);
    Parser_Reset(&g_parser);

    if(strlen(g_cfg.csv_path)>0){
    g_csv_fp = fopen(g_cfg.csv_path,"a");
    if(g_csv_fp){
            fprintf(g_csv_fp,"rpm_left,rpm_right,pitch,heartbeat,bad_frame,timeout,overflow\n");
            fflush(g_csv_fp);
    }
}

    serial_open(&g_ser, g_cfg.dev, g_cfg.baud);
    uint64_t last_send_ms = get_ms();
    uint32_t rx_bytes =0;
    uint64_t last_ok_ms = get_ms();      /* ← 新增 */
    uint64_t last_bad_tick = get_ms();   /* ← 新增 */

        while(1){
        uint64_t now_ms = get_ms();
        static int s_prev_conn = -1;
        if(s_prev_conn < 0)
        {
            s_prev_conn = g_ser.is_connected;       /* -1 表示还没初始化 */
        }
        else if(s_prev_conn && !g_ser.is_connected)
        {
            g_t_lost        = get_ms();
            g_t_reopen      = 0;
            g_t_first_frame = 0;
            fprintf(stderr,"[LOST ] t=%llu ms, device lost\n",
                    (unsigned long long)g_t_lost);
        }
        s_prev_conn = g_ser.is_connected;

        // ========== 1.帧字节间隔超时检测，和MCU对齐 FRAME_TIMEOUT_MS=50 ==========
        if(g_parser.state != WAIT_HEAD1)
        {
            if( (now_ms - g_parser.last_tick) > FRAME_TIMEOUT_MS )
            {
                Protocol_CountBad();
                Parser_Reset(&g_parser);
            }
        }

        fd_set rfds;
        FD_ZERO(&rfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 20000; //20ms

                if(g_ser.is_connected){
            FD_SET(g_ser.fd, &rfds);
            int ret = select(g_ser.fd+1, &rfds, NULL, NULL, &tv);
            if(ret < 0){
                g_t_lost        = get_ms();
                g_t_reopen      = 0;
                g_t_first_frame = 0;
                fprintf(stderr,"[LOST ] t=%llu ms, device lost\n",
                        (unsigned long long)g_t_lost);
                serial_close(&g_ser);
                g_ser.is_connected = 0;
                continue;
            }
            if(ret > 0 && FD_ISSET(g_ser.fd, &rfds)){
                uint8_t tmp[128];
                int n = serial_read(&g_ser, tmp, sizeof(tmp));
                if(n>0){
                    rx_bytes += n;
                    for(int i=0;i<n;i++){
                        RingBuf_Push(&g_rb, tmp[i]);
                    }
                }
                else if(n < 0){
                    if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR){
                        /* 非阻塞读暂无数据 / 被信号打断：不是拔线，什么都不做 */
                    }
                    else{
                        g_t_lost        = get_ms();
                        g_t_reopen      = 0;
                        g_t_first_frame = 0;
                        fprintf(stderr,"[LOST ] t=%llu ms, read error(%d)\n",
                                (unsigned long long)g_t_lost, errno);
                        serial_close(&g_ser);
                        g_ser.is_connected = 0;
                        continue;
                    }
                }
            }
        }   else{
            /* 断线重试：100ms 轮询，比 sleep(1) 快最多 900ms */
            struct timespec ts_retry = {0, 100 * 1000 * 1000};   /* 100ms = 1e8 ns */
            nanosleep(&ts_retry, NULL);
            if(serial_open(&g_ser, g_cfg.dev, g_cfg.baud) == 0)
            {
                g_t_reopen = get_ms();
                if(g_t_lost != 0)
                {
                    fprintf(stderr,"[REOPEN] t=%llu ms  (lost->open = %llu ms)\n",
                            (unsigned long long)g_t_reopen,
                            (unsigned long long)(g_t_reopen - g_t_lost));
                }
                else
                {
                    fprintf(stderr,"[REOPEN] t=%llu ms  (首次连接，无断开起点)\n",
                            (unsigned long long)g_t_reopen);
                }
            }
            else
            {
                continue;                 /* open 失败，下一轮再试 */
            }
            last_ok_ms    = get_ms();
            last_bad_tick = get_ms();
            continue;
        }
        uint64_t now = get_ms();
        if(g_cfg.enable_send && (now - last_send_ms >= SEND_PERIOD_MS)){
            uint8_t frame[128];
            int flen = Protocol_MakeSpeedFrame(frame, g_cfg.send_rpm1, g_cfg.send_rpm2);
            if(flen>0){
                serial_write(&g_ser, frame, flen);
            }
            last_send_ms = now;
        }

        // ==========2.从环形缓冲区取字节，调用解析，补齐第三个参数 ==========
        uint8_t ch;
        StatusFrame_t st;
        memset(&st,0,sizeof(st));
        while(RingBuf_Pop(&g_rb, &ch)==0){
            Protocol_ParseByte(&g_parser, ch, &st);

            //解析成功收到0x02状态帧
            if(st.valid == 1)
            {
                //终端打印
                last_ok_ms =get_ms();
                if(g_t_lost && g_t_first_frame == 0)
                {
                    g_t_first_frame = get_ms();
                    fprintf(stderr,
                        "\n[RECOVER] 拔线恢复总耗时 = %llu ms"
                        "   (open->首帧 = %llu ms)\n\n",
                        (unsigned long long)(g_t_first_frame - g_t_lost),
                        (unsigned long long)(g_t_first_frame - g_t_reopen));
                        g_t_lost =0;
                }
                printf("L_rpm:%d R_rpm:%d pitch:%d hb:%u bad:%u timeout:%u overflow:%u\n",
                    st.l_rpm, st.r_rpm, st.pitch, st.heartbeat,
                    st.bad_frame, st.timeout_cnt, st.overflow_cnt);

                //写入csv
                if(g_csv_fp != NULL)
                {
                    fprintf(g_csv_fp,"%d,%d,%d,%u,%u,%u,%u\n",
                        
                        st.l_rpm, st.r_rpm, st.pitch, st.heartbeat,
                        st.bad_frame, st.timeout_cnt, st.overflow_cnt);
                    fflush(g_csv_fp);
                }
                st.valid =0;
            }
        }

        //定时下发转速指令
        
            /* ==========3.每秒统计打印（与帧无关，错配时也看得见）========== */
        static uint64_t last_stat = 0;
        uint64_t t_stat = get_ms();
                /* 连续 50ms 一帧合法帧都没收到 = 一次链路异常，每 50ms 最多计一次 */
        uint64_t t_now = get_ms();
        if(t_now - last_ok_ms >= STALL_TIMEOUT_MS)
        {
            if(t_now - last_bad_tick >= STALL_TIMEOUT_MS)
            {
                
                last_bad_tick = t_now;
            }
        }
        if(t_stat - last_stat >= 1000)
        {
                printf("[STAT] baud=%d  rx=%u B/s  mcu_bad=%u  local_bad=%u  ok=%u\n",
                       g_cfg.baud, rx_bytes, st.bad_frame,
                       Protocol_GetBad(), Protocol_GetOk());
                fflush(stdout);
                rx_bytes = 0;                       // 新增，每秒清零
                last_stat = t_stat;
        }
    }

    if(g_csv_fp) fclose(g_csv_fp);
    serial_close(&g_ser);
    return 0;
}