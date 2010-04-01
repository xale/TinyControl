CC=gcc
LDFLAGS=-lpthread
CFLAGS=-std=c99 -pedantic -Wall -Wextra -O
SOURCES=TCServerMain.c TCServerSocket.c TCListenSocket.c TCTypes.c queue.c
DEPENDS=$(SOURCES:.c=.d)

EXECUTABLES=server
server_OBJECTS=TCServerMain.o TCServerSocket.o TCListenSocket.c TCTypes.o queue.o

.PHONY:all
all: $(DEPENDS) $(EXECUTABLES)

.SECONDEXPANSION:
$(EXECUTABLES): $$($$@_OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

-include $(DEPENDS)

.PHONY:debug
debug: CFLAGS:=$(CFLAGS) -g -O0
debug: all

.PHONY:clean
clean:
	$(RM) *.o *.d $(EXECUTABLES)

%.d: %.c
	$(SHELL) -ec "$(CC) -M $(CFLAGS) $< | sed 's/^$*.o/& $@/g' > $@"
