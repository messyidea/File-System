CC = gcc
CFLAGS = -g -w
SRCS = command.h  common.h  init.h  login.h  main.c  shell.h  structs.h

all: filesystem

filesystem: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o filesystem

clean:
	rm -f *.o
	rm -f filesystem
	rm -f disk
