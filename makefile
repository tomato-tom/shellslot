CC = gcc
CFLAGS = -Wall -Wextra

slot: main.o terminal_utils.o
	$(CC) $(CFLAGS) main.o terminal_utils.o -o slot

main.o: main.c terminal_utils.h
	$(CC) $(CFLAGS) -c main.c

terminal_utils.o: terminal_utils.c terminal_utils.h
	$(CC) $(CFLAGS) -c terminal_utils.c

clean:
	rm -f *.o slot
