SRCS = *.c *.h
CC = gcc
CFLAGS = -Wall -Wno-unused-variable -g -O2 -std=gnu11
LDFLAGS = -lpthread

all: build

build:
	$(CC) $(CFLAGS) -o main $(SRCS) $(LDFLAGS)

clean:
	rm -f *.o main