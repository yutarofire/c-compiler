CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./9cc tests/tests.c > tmp.s
	echo 'int char_fn() { return 257; } int static_fn() { return 5; }' | \
		gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

clean:
	rm -rf 9cc *.o *~ tmp* tests/*~ tests/*.o

.PHONY: test clean
