CFLAGS+=-g

.PHONY: all run_test clean
all : run_test

test: test.o
	$(CC) -g -o $@ $<

run_test: test
	./test

clean:
	rm test test.o

test.o: testdrive.h

