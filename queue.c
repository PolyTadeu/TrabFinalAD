#ifndef QUEUE_HEADER
#define QUEUE_HEADER
#include "event.c"

typedef enum _QueueType {
    Queue_FCFS, Queue_LCFS,
} QueueType;

typedef struct _Queue {
    QueueType type;
    int len;
    int head;
    int tail;
    Person *people;
} Queue;

Queue init_queue(QueueType type);
void deinit_queue(Queue *q);
int size_queue(Queue *q);
int is_empty_queue(Queue *q);
void insert_queue(Queue *q, Person p);
Person remove_queue(Queue *q);

#endif // QUEUE_HEADER

#ifdef QUEUE_IMPL
#undef QUEUE_IMPL
#import <stdlib.h>
#import <assert.h>

// inicializa a lista de eventos
Queue init_queue(QueueType type) {
    Queue ret = {
        .type = type,
        .len = 0,
        .head = 0,
        .tail = 0,
        .people = 0,
    };
    return ret;
}

void deinit_queue(Queue *q) {
    free(q->people);
}

int size_queue(Queue *q) {
    assert( q->type != Queue_LCFS || q->head == 0 );
    return ( q->tail - q->head + q->len ) % q->len;
}

int is_empty_queue(Queue *q) {
    assert( q->type != Queue_LCFS || q->head == 0 );
    return q->head == q->tail;
}

void insert_queue(Queue *q, Person p) {
    assert( q->type != Queue_LCFS || q->head == 0 );
    if ( q->len == 0 ) {
        q->len = 4;
        q->people = realloc(q->people, sizeof(*(q->people))*q->len);
        assert( q->people );
    } else if ( size_queue(q) + 1 >= q->len ) {
        q->len *= 2;
        q->people = realloc(q->people, sizeof(*(q->people))*q->len);
        assert( q->people );
        if ( q->head > q->tail ) {
            assert( q->type == Queue_FCFS );
            const int head_size = (q->len/2) - q->head;
            const int tail_size = q->tail;
            if ( head_size < tail_size ) {
                int new_head = q->head + (q->len/2);
                for ( int i = 0; i < head_size; i++ ) {
                    q->people[new_head + i] = q->people[q->head + i];
                }
                q->head = new_head;
            } else {
                for ( int i = 0; i < tail_size; i++ ) {
                    q->people[(q->len/2) + i] = q->people[i];
                }
                q->tail += q->len/2;
            }
        }
    }
    q->people[q->tail] = p;
    q->tail = ( q->tail + 1 ) % q->len;
}

Person remove_queue(Queue *q) {
    assert( !is_empty_queue(q) );
    Person ret;
    switch ( q->type ) {
        case Queue_FCFS: {
            ret = q->people[q->head];
            q->head = ( q->head + 1 ) % q->len;
        } break;
        case Queue_LCFS: {
            assert( q->head == 0 );
            q->tail = ( q->tail + q->len - 1 ) % q->len;
            ret = q->people[q->tail];
        } break;
        default: {
            assert( 0 && "unreachable" );
        } break;
    }
    return ret;
}

#ifdef QUEUE_MAIN
#undef QUEUE_MAIN
#include <stdio.h>

void print_queue(Queue *q) {
    printf("{");
    for ( int i = 0; i < q->len; i++ ) {
        printf("%s%s%02u", (i == q->head) ? " [" : " ", (i == q->tail) ? "]" : "", q->people[i].color);
    }
    printf(" }");
}

void test_queue(Queue *q, Color color_start, Color color_end) {
    for ( Color color = color_start; color < color_end; color++ ) {
        Person p = {
            .arrived_time = 0,
            .color = color,
        };
        insert_queue(q, p);
        print_queue(q);
        printf("\n\n\n");
    }
    printf("=========\n");
    while ( !is_empty_queue(q) ) {
        Person p = remove_queue(q);
        printf("removed: %u\n", p.color);
        print_queue(q);
        printf("\n\n\n");
    }
}

int main() {
    Queue fcfs = init_queue(Queue_FCFS), *q1 = &fcfs;
    printf("FCFS:\n");
    test_queue(q1, 0, 10);
    test_queue(q1, 10, 30);
    deinit_queue(q1);

    Queue lcfs = init_queue(Queue_LCFS), *q2 = &lcfs;
    printf("LCFS\n");
    test_queue(q2, 50, 60);
    deinit_queue(q2);
}

#endif // QUEUE_MAIN

#endif // QUEUE_IMPL