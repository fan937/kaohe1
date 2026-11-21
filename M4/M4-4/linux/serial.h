#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <termios.h>

typedef struct {
    int fd;
    char dev[128];
    int baud;
    int is_connected;
} SerialDev_t;

int serial_open(SerialDev_t *s, const char *dev, int baudrate);
void serial_close(SerialDev_t *s);
int serial_read(SerialDev_t *s, uint8_t *buf, int len);
int serial_write(SerialDev_t *s, const uint8_t *buf, int len);

#endif