SRCS = *.c *.h
CC = gcc
CFLAGS = -Wall -Wno-unused-variable -g -O2 -std=gnu11
LDFLAGS = -lpthread

all: build

build:
	$(CC) $(CFLAGS) -o tema2 $(SRCS) $(LDFLAGS)

clean:
	rm -f *.o tema2