#ifndef SYSTEM_HEADER
#define SYSTEM_HEADER
#include "types.h"
#include "random.c"
#include "stats.c"
#include "event_heap.c"
#include "queue.c"

typedef struct _System {
    RandCtx *rand;
    f64 lambda;
    EventHeap *events;
    Queue *queue;
    b32 busy;
    Color color;
    Time curr_time;
    Stats nq_stat;
    Stats nq_stat_time;
    Stats tq_stat;
} System;

System init_system(RandCtx *rand, f64 lambda, EventHeap *eh, Queue *q, Color color);

#endif // SYSTEM_HEADER

#ifdef SYSTEM_IMPL
#undef SYSTEM_IMPL
#include <assert.h>

#define STATS_IMPL
#include "stats.c"
#define RAND_IMPL
#include "random.c"
#define EVENT_IMPL
#include "event.c"
#define EVENT_HEAP_IMPL
#include "event_heap.c"
#define QUEUE_IMPL
#include "queue.c"

System init_system(RandCtx *rand, f64 lambda, EventHeap *es, Queue *q, Color color) {
    const System ret = {
        .rand = rand,
        .lambda = lambda,
        .events = es,
        .queue = q,
        .busy = 0,
        .color = color,
        .curr_time = 0,
        .nq_stat = new_stats(),
        .nq_stat_time = new_stats(),
        .tq_stat = new_stats(),
    };
    return ret;
}

void record_queue_time(Stats *stat, Color color, Time now, Person p) {
    if ( p.color == color ) {
        const f64 val = now - p.arrived_time;
        acc_and_update(stat, val);
    }
}

void record_queue_size(System *s, Time now, Time last_time, u32 qsize) {
    acc_and_update(&(s->nq_stat), qsize);
    const f64 val = (now - last_time) * qsize;
    acc_and_update(&(s->nq_stat_time), val);
}

void handle_next_event(System *s) {
    Event e = remove_heap(s->events);
    record_queue_size(s, e.time, s->curr_time, size_queue(s->queue));
    switch ( e.type ) {
        case EVENT_arrival: {
            const Time t_arr = randExp(s->rand, s->lambda);
            const Event e_arr = create_event_arrival(t_arr, s->color);
            const Person p_arr = e_arr.person;
            if ( s->busy ) {
                insert_queue(s->queue, p_arr);
            } else {
                assert( e.time == p_arr.arrived_time );
                record_queue_time(&(s->tq_stat), s->color, e.time, p_arr);
                const Time t_lea = randExp(s->rand, s->lambda);
                const Event e_lea = create_event_leave(t_lea, p_arr);
                insert_heap(s->events, e_lea);
                s->busy = 1;
            }
        } break;
        case EVENT_leave: {
            assert( s->busy );
            if ( is_empty_queue(s->queue) ) {
                s->busy = 0;
            } else {
                const Person new_p_lea = remove_queue(s->queue);
                record_queue_time(&(s->tq_stat), s->color, e.time, new_p_lea);
                const Time new_t_lea = randExp(s->rand, s->lambda);
                const Event new_e_lea = create_event_leave(new_t_lea, new_p_lea);
                insert_heap(s->events, new_e_lea);
            }
        } break;
        default: {
            assert( 0 && "Unreachable" );
        } break;
    }
    s->curr_time = e.time;
}

#ifdef SYSTEM_MAIN
#undef SYSTEM_MAIN

int main() {
}

#endif // SYSTEM_MAIN

#endif // SYSTEM_IMPL
