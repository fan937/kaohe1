TARGET := build/serial_tool
SRCS := main.c serial.c ringbuf.c protocol.c
OBJS := $(SRCS:%.c=build/%.o)
CC := gcc
CFLAGS := -Wall -g -std=c99 -lrt

all: $(TARGET)

build:
	mkdir -p build

$(TARGET): build $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build