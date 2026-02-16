CC = clang
CFLAGS = -Wall -Wextra -g

all: parent child

parent: src/parent.c
	$(CC) $(CFLAGS) -o parent src/parent.c

child: src/child.c
	$(CC) $(CFLAGS) -o child src/child.c

clean:
	rm -f parent child
	rm -f out.txt
	rm -rf *.dSYM

run:
	./parent