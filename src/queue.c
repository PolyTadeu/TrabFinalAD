#ifndef QUEUE_HEADER
#define QUEUE_HEADER
#include "types.h"
#include "event.c"

//Enum para tipos das queues
typedef enum _QueueType {
    Queue_FCFS, Queue_LCFS,
} QueueType;

//Estrutura da queueu
typedef struct _Queue {
    QueueType type;
    u32 len;
    u32 head;
    u32 tail;
    Person *people;
} Queue;

Queue init_queue(QueueType type);
void deinit_queue(Queue *q);
u32 size_queue(const Queue *q);
b32 is_empty_queue(const Queue *q);
void insert_queue(Queue *q, const Person p);
Person remove_queue(Queue *q);

#endif // QUEUE_HEADER

#ifdef QUEUE_IMPL
#undef QUEUE_IMPL
#include <stdlib.h>
#include <assert.h>

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

//Libera memoria
void deinit_queue(Queue *q) {
    free(q->people);
}

//Retorna tamanho da queue
u32 size_queue(const Queue *q) {
    assert( q->type != Queue_LCFS || q->head == 0 );
    return q->len
        ? ( q->tail - q->head + q->len ) % q->len
        : 0;
}

//Checa se queue esta vazia
b32 is_empty_queue(const Queue *q) {
    assert( q->type != Queue_LCFS || q->head == 0 );
    return q->head == q->tail;
}

//Inserir na queue
void insert_queue(Queue *q, const Person p) {
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
            const u32 head_size = (q->len/2) - q->head;
            const u32 tail_size = q->tail;
            if ( head_size < tail_size ) {
                const u32 new_head = q->head + (q->len/2);
                for ( u32 i = 0; i < head_size; i++ ) {
                    q->people[new_head + i] = q->people[q->head + i];
                }
                q->head = new_head;
            } else {
                for ( u32 i = 0; i < tail_size; i++ ) {
                    q->people[(q->len/2) + i] = q->people[i];
                }
                q->tail += q->len/2;
            }
        }
    }
    q->people[q->tail] = p;
    q->tail = ( q->tail + 1 ) % q->len;
}

//Remover da queue
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

#include "test.c"

//Funcao para teste de pessoa
b32 person_expect_equal(const char *test_name,
        const Person expected, const Person actual) {
    b32 ret = 1;
    ret = ret
        && f64_expect_equal_tol(test_name,
            expected.arrived_time, actual.arrived_time, 0);
    ret = ret
        && u32_expect_equal(test_name, expected.color, actual.color);
    return ret;
}

//Funcao para testar o final da queue
b32 queue_expect_at_end(const Queue *q, const Person expected_last) {
    assert( !is_empty_queue(q) );
    const u32 last_i = (q->tail - 1 + q->len) % q->len;
    const Person actual_last = q->people[last_i];
    return person_expect_equal("Queue person at end",
            expected_last, actual_last);
}

//Funcao para logar a queuue
void log_queue(Queue *q) {
    log("{");
    for ( u32 i = 0; i < q->len; i++ ) {
        log("%s%s%02u", (i == q->head) ? " [" : " ", (i == q->tail) ? "]" : "", q->people[i].color);
    }
    log(" }\n");
}

//Funcaoa para testar queue
void test_queue(const char *test_name,
        Queue *q, Color color_start, Color color_end) {
    SECTIONn(test_name);
    nSUBSECTION("Queue Insert");
    for ( Color color = color_start; color < color_end; color++ ) {
        Person p = {
            .arrived_time = ((f64) color) * 1.5,
            .color = color,
        };
        insert_queue(q, p);
        log_queue(q);
        queue_expect_at_end(q, p);
        u32_expect_equal("Queue inserting increases size",
                color - color_start + 1, size_queue(q));
    }
    SUBSECTION("Queue Remove");
    const u32 len = color_end - color_start;
    u32 removed_cnt = 0;
    while ( !is_empty_queue(q) ) {
        Person p = remove_queue(q);
        removed_cnt += 1;
        log("removed: %u\n", p.color);
        log_queue(q);
        u32_expect_equal("Queue removing decreases size",
                len - removed_cnt, size_queue(q));
    }
    u32_expect_equal("Queue removed all that was put",
            len, removed_cnt);
    u32_expect_equal("Queue is empty",
            1, is_empty_queue(q));
}

//Main para rodar testes
int main() {
    Queue fcfs = init_queue(Queue_FCFS), *q1 = &fcfs;
    test_queue("FCFS 1", q1, 0, 10);
    test_queue("FCFS 2", q1, 10, 30);
    deinit_queue(q1);

    end_tests("Queue FCFS");

    Queue lcfs = init_queue(Queue_LCFS), *q2 = &lcfs;
    test_queue("LCFS", q2, 50, 60);
    deinit_queue(q2);

    end_tests("Queue LCFS");
}

#endif // QUEUE_MAIN

#endif // QUEUE_IMPL
