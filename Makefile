# ==== Compiler and command-line options and flags ====
CC=gcc

CFLAGS=-std=c99 -O0 -g -Wall -Wextra

# ==== Dependencies ====

OBJECTS=TCServerMain.o TCServerSocket.o TCListenSocket.c TCTypes.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ==== Build rules ====
ALL=main
all: $(ALL)

main: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o server

# ==== Clean rules ====
clean:
	rm -rf *.gch *.dSYM *.o
	
clean-all:
	make clean
	rm -f server
