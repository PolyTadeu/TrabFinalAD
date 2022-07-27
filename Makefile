CFLAGS=-Wall -Wextra -Werror
MATHLIB=-lm
DEF=-D $(join $1,_IMPL) -D $(join $1,_MAIN)

all: main

test_rand: random.c stats.c types.h
	$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, RAND) random.c
	./a.out

test_event_heap: event_heap.c event.c types.h
	$(CC) $(CFLAGS) $(call DEF, EVENT_HEAP) event_heap.c
	./a.out

test_queue: event.c queue.c types.h
	$(CC) $(CFLAGS) $(call DEF, QUEUE) queue.c
	./a.out

test_histogram: histogram.c types.h
	$(CC) $(CFLAGS) $(call DEF, HISTOGRAM) histogram.c
	./a.out

test_system: event_heap.c event.c queue.c random.c stats.c system.c types.h
	$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, SYSTEM) system.c
	./a.out

main: main.c event_heap.c event.c queue.c random.c types.h
	$(CC) $(CFLAGS) $(CLIBS) main.c -o main

clean:
	rm -f *.out

