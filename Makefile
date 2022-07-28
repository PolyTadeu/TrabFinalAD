CFLAGS=-Wall -Wextra -Werror
MATHLIB=-lm
DEF=-D $(join $1,_IMPL) -D $(join $1,_MAIN)
SRCDIR=src/

all: main

test_rand: $(SRCDIR)random.c $(SRCDIR)stats.c $(SRCDIR)types.h
	$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, RAND) $(SRCDIR)random.c
	./a.out

test_event_heap: $($(SRCDIR){event_heap.c, event.c, types.h})
	$(CC) $(CFLAGS) $(call DEF, EVENT_HEAP) $(SRCDIR)event_heap.c
	./a.out

test_queue: $($(SRCDIR){event.c, queue.c, types.h})
	$(CC) $(CFLAGS) $(call DEF, QUEUE) $(SRCDIR)queue.c
	./a.out

test_system: $($(SRCDIR){event_heap.c, event.c, queue.c, random.c, stats.c, system.c, types.h})
	$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, SYSTEM) $(SRCDIR)system.c
	./a.out

main: $($(SRCDIR){main.c, event_heap.c, event.c, queue.c, random.c, types.h})
	$(CC) $(CFLAGS) $(CLIBS) $(SRCDIR)main.c -o main

clean:
	rm -f *.out

