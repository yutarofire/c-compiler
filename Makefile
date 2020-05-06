CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

chibicc: $(OBJS)
	$(CC) -o chibicc $(OBJS) $(LFLAGS)

$(OBJS): chibicc.h

test: chibicc
	./test.sh

clean:
	rm -f chibicc *.o

.PHONY: test clean
