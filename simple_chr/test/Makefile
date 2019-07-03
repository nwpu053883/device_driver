CC=gcc
CFLAGS=-g -Wall
LDFLAGS=

target=gmem_demo
objs=$(patsubst %.c, %.o, $(wildcard *.c))

all:$(target)

$(target):$(objs)
	$(CC) $^ -o $@ $(LDFLAGS)

.c.o:
	$(CC) -c $< $(CFLAGS)

.PHONY:
	clean

clean:
	rm *.o $(target) -rf
