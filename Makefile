CC=gcc
LDFLAGS=-lpthread
CFLAGS=-std=gnu99 -pedantic -Wall -Wextra -O
SOURCES=TCServerMain.c TCServerSocket.c TCListenSocket.c TCTypes.c TCUtilities.c queue.c reader.c client.c
DEPENDS=$(SOURCES:.c=.d)

EXECUTABLES=server client
server_OBJECTS=TCServerMain.o TCServerSocket.o TCListenSocket.o TCTypes.o TCUtilities.o queue.o
client_OBJECTS=queue.o reader.o TCTypes.o client.o TCUtilities.o

.PHONY:all
all: $(DEPENDS) $(EXECUTABLES)

.SECONDEXPANSION:
$(EXECUTABLES): $$($$@_OBJECTS)
	$(CC) $(LDFLAGS) $(CFLAGS) $^ -o $@

-include $(DEPENDS)

.PHONY:debug
debug: CFLAGS+=-g -O0
debug: all

.PHONY:profile
profile: CFLAGS+=-pg -g -fprofile-arcs -ftest-coverage
profile: all

.PHONY:clean
clean:
	$(RM) *.o *.d *.out *.gcov *.gcda *.gcno tstfile $(EXECUTABLES)

%.d: %.c
	$(SHELL) -ec "$(CC) -M $< | sed 's/^$*.o/& $@/g' > $@"
