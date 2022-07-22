#ifndef EVENT_HEADER
#define EVENT_HEADER

typedef struct _Event {
    double time;
} Event;

typedef struct _EventQueue {
    int size, len;
    Event* events;
} EventQueue;

void insert_heap(EventQueue *q, Event e);
Event remove_heap(EventQueue *q);

#endif // EVENT_HEADER

#ifdef EVENT_IMPL
#import <stdlib.h>
#import <assert.h>

EventQueue init() {
    EventQueue ret = {
        .size = 0, 
        .len = 0,
        .events = 0,
    };
    return ret;
}

#define MIN(a, b)   (((a) <= (b) ? (a) : (b)))
#define PARENT(i)   (((i) - 1) / 2)
#define LEFT(i)     ((i) * 2 + 1)
#define RIGHT(i)    ((i) * 2 + 2)

void bubbleUp(EventQueue *q, int i) {
    Event *events = q->events;
    while (i > 0 && events[PARENT(i)].time > events[i].time ) {
       const Event tmp = events[PARENT(i)];
       events[PARENT(i)] =  events[i];
       events[i] = tmp;
       i = PARENT(i);
    }
}

void bubbleDown(EventQueue *q, int i) {
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

void insert_heap(EventQueue *q, Event e) {
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

Event remove_heap(EventQueue *q) {
    assert( q->size );
    const Event ret = q->events[0];
    q->size -= 1;
    q->events[0] = q->events[q->size];
    if ( q->size )
        bubbleDown(q, 0);
    return ret;
}

#undef MIN
#undef PARENT
#undef LEFT
#undef RIGHT

#ifdef EVENT_MAIN
#include <stdio.h>

void padd_x() {
    const int half_fixed_pad = 1;
    for ( int j = 0; j < half_fixed_pad; j++ )
        printf("  ");
}

void padd_y(int dim) {
    const int half_y_pad = 2;
    for ( int i = 0; i < half_y_pad; i++ )
        printf("  ");
    for ( int i = 0; i < dim; i++ )
        padd_x();
}

void print_heap(EventQueue *q) {
    int depth = 0; (void) depth;
    int size = q->size;
    int dim_pad = 0;
    while ( size ) {
        size /= 2;
        depth += 1;
        dim_pad = dim_pad * 2 + 1;
    }

    int next_start = 0;
    int line = 1;
    int next_line = 0;
    for ( int i = 0; i < q->size; i++ ) {
        if ( next_start != i ) {
            padd_x();
            next_start += line;
        }
        padd_y(dim_pad);

        printf("%4.2lf", q->events[i].time);
        if ( next_line == i ) {
            printf("\n");
            line *= 2;
            next_line += line;
        } else {
            padd_y(dim_pad);
        }
    }
}

int main() {
    EventQueue eq = init(), *q = &eq;
    double arr[] = { 5, 9, 14, 17, 1, 3 };
    for ( int i = 0; i < 6; i++ ) {
        const Event e = {
            .time = arr[i],
        };
        insert_heap(q, e);

        print_heap(q);
        printf("\n\n\n");
    }
    printf("=========\n\n");
    while ( q->size ) {
        const Event e = remove_heap(q);
        printf("removed: %4.2lf\n", e.time);
        print_heap(q);
        printf("\n\n\n");        
    }
}

#endif // EVENT_MAIN

#endif // EVENT_IMPL
