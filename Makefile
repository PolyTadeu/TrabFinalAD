CFLAGS=-Wall -Wextra -Werror
MATHLIB=-lm
DEF=-D $(join $1,_IMPL) -D $(join $1,_MAIN)
SRC=src/
VERB=-D VERBOSE

all: main_exe

test_all_v: test_rand_v test_event_heap_v test_queue_v test_system_v

test_all: test_rand test_event_heap test_queue test_system

test_rand_v: $($(SRC){random.c ,stats.c, test.c, types.h})
	$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, RAND) $(VERB) $(SRC)random.c
	./a.out

test_rand: $($(SRC)random.c, stats.c, test.c, types.h})
	@$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, RAND) $(SRC)random.c
	@./a.out

test_event_heap_v: $($(SRC){event_heap.c, event.c, test.c, types.h})
	$(CC) $(CFLAGS) $(call DEF, EVENT_HEAP) $(VERB) $(SRC)event_heap.c
	./a.out

test_event_heap: $($(SRC){event_heap.c, event.c, test.c, types.h})
	@$(CC) $(CFLAGS) $(call DEF, EVENT_HEAP) $(SRC)event_heap.c
	@./a.out

test_queue_v: $($(SRC){event.c, queue.c, types.h})
	$(CC) $(CFLAGS) $(call DEF, QUEUE) $(VERB) $(SRC)queue.c
	./a.out

test_queue: $($(SRC){event.c, queue.c, types.h})
	@$(CC) $(CFLAGS) $(call DEF, QUEUE) $(SRC)queue.c
	@./a.out

test_system_v: $($(SRC){event_heap.c, event.c, queue.c, random.c, stats.c, system.c, test.c, types.h})
	$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, SYSTEM) $(VERB) $(SRC)system.c
	./a.out

test_system: $($(SRC){event_heap.c, event.c, queue.c, random.c, stats.c, system.c, test.c, types.h})
	@$(CC) $(CFLAGS) $(MATHLIB) $(call DEF, SYSTEM) $(SRC)system.c
	@./a.out

main_exe: $($(SRC){args.c, main.c, event_heap.c, event.c, queue.c, random.c, stats.c, system.c, types.h})
	$(CC) $(CFLAGS) $(MATHLIB) $(SRC)main.c -o main

clean:
	rm -f *.out main

