#ifndef EVENT_HEAP_HEADER
#define EVENT_HEAP_HEADER
#include "types.h"
#include "event.c"

typedef struct _EventHeap {
    u32 size, len;
    Event* events;
} EventHeap;

EventHeap init_heap();
void deinit_heap(EventHeap *q);
u32 size_heap(const EventHeap *q);
b32 is_empty_heap(const EventHeap *q);
void insert_heap(EventHeap *q, Event e);
Event remove_heap(EventHeap *q);

void offset_heap_events_by(EventHeap *q, f64 offset);
void inc_next_arrival_event_color(EventHeap *q);

#endif // EVENT_HEAP_HEADER

#ifdef EVENT_HEAP_IMPL
#undef EVENT_HEAP_IMPL
#include <stdlib.h>
#include <assert.h>

// inicializa a lista de eventos
EventHeap init_heap() {
    EventHeap ret = {
        .size = 0,
        .len = 0,
        .events = 0,
    };
    return ret;
}

void deinit_heap(EventHeap *q) {
    free(q->events);
}

u32 size_heap(const EventHeap *q) {
    return q->size;
}

b32 is_empty_heap(const EventHeap *q) {
    return q->size == 0;
}

#define MIN(a, b)   (((a) <= (b) ? (a) : (b)))
#define PARENT(i)   (((i) - 1) / 2)
#define LEFT(i)     ((i) * 2 + 1)
#define RIGHT(i)    ((i) * 2 + 2)

// função utilizada para ordenar a lista quando há uma inserção 
void bubbleUp(EventHeap *q, u32 i) {
    Event *events = q->events;
    while (i > 0 && events[PARENT(i)].time > events[i].time ) {
       const Event tmp = events[PARENT(i)];
       events[PARENT(i)] =  events[i];
       events[i] = tmp;
       i = PARENT(i);
    }
}

// função utilizada para ordenar a lista quando há uma remoção
void bubbleDown(EventHeap *q, u32 i) {
    Event *events = q->events;
    assert( i < q->size );
    while ( ( LEFT(i) < q->size && events[LEFT(i)].time < events[i].time ) || ( RIGHT(i) < q->size && events[RIGHT(i)].time < events[i].time ) ) {
        if ( RIGHT(i) >= q->size || events[LEFT(i)].time < events[RIGHT(i)].time ) {
            const Event tmp = events[LEFT(i)];
            events[LEFT(i)] = events[i];
            events[i] = tmp;
            i = LEFT(i);
        } else {
            const Event tmp = events[RIGHT(i)];
            events[RIGHT(i)] = events[i];
            events[i] = tmp;
            i = RIGHT(i);
        }
    }
}

// insere evento na lista
void insert_heap(EventHeap *q, Event e) {
    if ( q->size + 1 > q->len ) {
        assert( q->size == q->len );
        q->len = (q->len) ? (q->len * 2) : 4;
        q->events = realloc(q->events, sizeof(*(q->events))*q->len);
        assert( q->events );
    }
    q->events[q->size] = e;
    bubbleUp(q, q->size);
    q->size += 1;
}

// remove evento da lista
Event remove_heap(EventHeap *q) {
    assert( q->size );
    const Event ret = q->events[0];
    q->size -= 1;
    q->events[0] = q->events[q->size];
    if ( q->size )
        bubbleDown(q, 0);
    return ret;
}

void offset_heap_events_by(EventHeap *q, f64 offset) {
    assert( offset >= 0 );
    for ( u32 i = 0; i < q->size; i++ ) {
        assert( q->events[i].time >= offset );
        q->events[i].time -= offset;
        q->events[i].person.arrived_time -= offset;
    }
}

void inc_next_arrival_event_color(EventHeap *q) {
    for ( u32 i = 0; i < q->size; i++ ) {
        Event *e = &(q->events[i]);
        if ( e->type == EVENT_arrival ) {
            e->person.color += 1;
            return;
        }
    }
    assert( 0 && "arrival_event not found" );
}

#ifdef EVENT_HEAP_MAIN
#undef EVENT_HEAP_MAIN

#include "test.c"

void half_padd_x() {
    log("  ");
}

void padd_x() {
    half_padd_x();
    half_padd_x();
}

void padd_y(u32 dim) {
    for ( u32 i = 0; i < dim; i++ ) {
        log("   ");
        half_padd_x();
    }
}

void log_heap(EventHeap *q) {
    u32 depth = 0;
    u32 size = q->size;
    u32 dim_pad = 0;
    while ( size ) {
        size /= 2;
        depth += 1;
        dim_pad = dim_pad * 2 + 1;
    }

    u32 next_start = 0;
    u32 line = 1;
    u32 next_line = 0;
    dim_pad /= 2;
    for ( u32 i = 0; i < q->size; i++ ) {
        if ( next_start != i ) {
            padd_x();
        } else {
            next_start += line;
        }
        padd_y(dim_pad);

        log("%6.2lf", q->events[i].time);
        if ( next_line == i ) {
            log("\n");
            line *= 2;
            next_line += line;
            dim_pad /= 2;
        } else {
            padd_y(dim_pad);
        }
    }
    log("\n");
}

void heap_expect_ok(const EventHeap *q) {
    const Event *events = q->events;
    for ( u32 i = 0; i < q->size; i++ ) {
        if ( LEFT(i) < q->size ) {
            f64_expect_less_eq("Heap root <= left",
                    events[i].time, events[LEFT(i)].time);
        }
        if ( RIGHT(i) < q->size ) {
            f64_expect_less_eq("Heap root <= right",
                    events[i].time, events[RIGHT(i)].time);
        }
    }
}

int main() {
    SECTION("Heap Insert");
    EventHeap eq = init_heap(), *q = &eq;
    const u32 arr_len = 7;
    f64 arr[] = { 5, 9, 14, 17, 1, 3, 7 };
    for ( u32 i = 0; i < arr_len; i++ ) {
        const Event e = {
            .time = arr[i],
        };
        log("\ninserting: %4.2lf\n", e.time);
        insert_heap(q, e);
        log_heap(q);
        heap_expect_ok(q);
        u32_expect_equal("Heap inserting increases size",
                i+1, size_heap(q));
    }

    SECTION("Heap Remove");
    u32 removed_cnt = 0;
    while ( size_heap(q) ) {
        const Event e = remove_heap(q);
        removed_cnt += 1;
        log("\nremoved: %4.2lf\n", e.time);
        log_heap(q);
        heap_expect_ok(q);
        u32_expect_equal("Heap removing decreases size",
                arr_len - removed_cnt, size_heap(q));
    }
    u32_expect_equal("Heap removed all that was put",
            arr_len, removed_cnt);
    u32_expect_equal("Heap is empty",
            0, size_heap(q));
    deinit_heap(q);

    end_tests("Heap");
}

#endif // EVENT_HEAP_MAIN

#endif // EVENT_HEAP_IMPL

#undef MIN
#undef PARENT
#undef LEFT
#undef RIGHT
