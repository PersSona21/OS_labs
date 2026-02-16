CC = clang
CFLAGS = -Wall -Wextra -g -pthread

all: parallel sequential

parallel: src/parallel.c
	$(CC) $(CFLAGS) -o parallel src/parallel.c

sequential: src/sequential.c
	$(CC) $(CFLAGS) -o sequential src/sequential.c
	
clean:
	rm -f parallel sequential
	rm -rf *.dSYM

run:
	./parallel 512 64 1
	./parallel 512 64 2
	./parallel 512 64 4
	./parallel 512 64 6
	./parallel 512 64 8
	./parallel 512 64 16
	./parallel 512 64 64
	./parallel 512 64 128
	./parallel 512 64 512
	