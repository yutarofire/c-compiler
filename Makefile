CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

occ: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): occ.h

test: occ
	./occ tests/tests.c > tmp.s
	echo 'int char_fn() { return 257; } int static_fn() { return 5; }' | \
		gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

clean:
	rm -rf occ *.o *~ tmp* tests/*~ tests/*.o

.PHONY: test clean
