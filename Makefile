CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -O
SOURCES=TCServerMain.c TCServerSocket.c TCListenSocket.c TCTypes.c queue.c
EXECUTABLES=server
DEPENDS=$(SOURCES:.c=.d)
SERVER_OBJECTS=TCServerMain.o TCServerSocket.o TCListenSocket.c TCTypes.o queue.o

.PHONY:all
all: $(DEPENDS) $(EXECUTABLES)

server: $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

-include $(DEPENDS)

.PHONY:debug
debug: CFLAGS:=$(CFLAGS) -g -O0
debug: all

.PHONY:clean
clean:
	$(RM) *.o *.d $(EXECUTABLES)

%.d: %.c
	$(SHELL) -ec "$(CC) -M $(CPPFLAGS) $< | sed 's/^$*.o/& $@/g' > $@"
