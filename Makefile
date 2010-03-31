CFLAGS=-std=c99 -pedantic -Wall -Wextra -O
SOURCES=queue.c
EXECUTABLES=
DEPENDS=$(SOURCES:.c=.d)
OBJECTS=$(SOURCES:.c=.o)

.PHONY:all
all: $(DEPENDS) $(EXECUTABLES)

-include $(DEPENDS)

.PHONY:debug
debug: CFLAGS:=$(CFLAGS) -g -O0
debug: all

.PHONY:clean
clean:
	$(RM) *.o *.d $(EXECUTABLES)

%.d: %.c
	$(SHELL) -ec "$(CC) -M $(CPPFLAGS) $< | sed 's/^$*.o/& $@/g' > $@"
