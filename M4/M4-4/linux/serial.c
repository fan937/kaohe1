#include "serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

static int baud_to_const(int baud)
{
    switch(baud){
        case 9600: return B9600;
        case 115200: return B115200;
        case 57600: return B57600;
        default: return B115200;
    }
}

int serial_open(SerialDev_t *s, const char *dev, int baudrate)
{
    serial_close(s);
    strncpy(s->dev, dev, sizeof(s->dev)-1);
    s->baud = baudrate;

    int fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd < 0){
        perror("open serial");
        s->is_connected = 0;
        return -1;
    }

    struct termios t;
    memset(&t, 0, sizeof(t));
    if(tcgetattr(fd, &t) < 0){
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    // 手动实现cfmakeraw，无GNU扩展依赖
    t.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    t.c_oflag &= ~OPOST;
    t.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    t.c_cflag &= ~(CSIZE|PARENB);
    t.c_cflag |= CS8;

    speed_t spd = baud_to_const(baudrate);
    cfsetispeed(&t, spd);
    cfsetospeed(&t, spd);

    if(tcsetattr(fd, TCSANOW, &t) < 0){
        perror("tcsetattr");
        close(fd);
        return -1;
    }
    tcflush(fd, TCIOFLUSH);

    s->fd = fd;
    s->is_connected = 1;
    return 0;
}

void serial_close(SerialDev_t *s)
{
    if(s->fd > 0){
        close(s->fd);
    }
    s->fd = -1;
    s->is_connected = 0;
}

int serial_read(SerialDev_t *s, uint8_t *buf, int len)
{
    if(!s->is_connected || s->fd <= 0) return -1;
    int ret = read(s->fd, buf, len);
    if(ret <= 0){
        if(ret == 0 || errno != EAGAIN){
            serial_close(s);
        }
        return ret;
    }
    return ret;
}

int serial_write(SerialDev_t *s, const uint8_t *buf, int len)
{
    if(!s->is_connected || s->fd <=0) return -1;
    return write(s->fd, buf, len);
}