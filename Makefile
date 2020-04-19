CFLAGS=-std=c11 -g -static

9cc: 9cc.c

test: 9cc
	./test.sh
	rm 9cc

clean:
	rm 9cc

.PHONY: test clean
