#ifndef SYSTEM_HEADER
#define SYSTEM_HEADER
#include "types.h"
#include "random.c"
#include "stats.c"
#include "event_heap.c"
#include "queue.c"

//Estrutura para o  sistema
typedef struct _System {
    RandCtx *rand;
    f64 lambda;
    f64 mu;
    EventHeap *events;
    Queue *queue;
    b32 busy;
    Color color;
    Time curr_time;
    Stats nq_stat;
    Stats wt_stat;
} System;

System init_system(RandCtx *rand, f64 lambda, f64 mu, EventHeap *eh, Queue *q, Color color);
void deinit_system(System *s);
b32 is_empty_system(const System *s);
void add_first_event(System *s);
void next_batch(System *s);
Event handle_next_event(System *s);

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

//Cria uma estrutura sistema
System init_system(RandCtx *rand, f64 lambda, f64 mu, EventHeap *eh, Queue *q, Color color) {
    const System ret = {
        .rand = rand,
        .lambda = lambda,
        .mu = mu,
        .events = eh,
        .queue = q,
        .busy = 0,
        .color = color,
        .curr_time = 0,
        .nq_stat = new_stats(),
        .wt_stat = new_stats(),
    };
    return ret;
}

//Destroi o sistema
//Abrindo a memoria da heap de evento e da fila de espera
void deinit_system(System *s) {
    deinit_heap(s->events);
    deinit_queue(s->queue);
}

//Funcao para checar se o sistema esta vazio
b32 is_empty_system(const System *s) {
    return is_empty_heap(s->events)
        && is_empty_queue(s->queue)
        && s->curr_time == 0
        && s->nq_stat.n == 0
        && s->wt_stat.n == 0;
}

//Cria oo primeiro evento do sistema
void add_first_event(System *s) {
    assert( is_empty_system(s) );
    const Event e_arr = create_event_arrival(0.0, s->color);
    insert_heap(s->events, e_arr);
}

//Funcao para gerar proxima rodada
void next_batch(System *s) {
    assert( s->color + 1 != 0 );
    offset_heap_events_by(s->events, s->curr_time);
    inc_next_arrival_event_color(s->events);
    System new_sys = {
        .rand = s->rand,
        .lambda = s->lambda,
        .mu = s->mu,
        .events = s->events,
        .queue = s->queue,
        .busy = s->busy,
        .color = s->color + 1,
        .curr_time = 0,
        .nq_stat = new_stats(),
        .wt_stat = new_stats(),
    };
    *s = new_sys;
}

//Registra o tempo de espera de um cliente
void record_wait_time(Stats *stat,
        const Color color, const Time now, const Person p) {
    if ( p.color == color ) {
        const f64 val = now - p.arrived_time;
        assert( val >= 0 );
        acc_and_update(stat, val, 1);
    }
}
//Registra o tamanho da fila de espera
void record_queue_size(Stats *stat,
        const Time now, Time last_time, u32 qsize) {
    acc_and_update(stat, qsize, (now - last_time));
}

//Funcao para tratar o proximo evento na heap de eventos
Event handle_next_event(System *s) {
    const Event e = remove_heap(s->events);
    const Time now = e.time;
    const Time last_time = s->curr_time;
    assert( last_time <= now );
    const Person p = e.person;
    record_queue_size(&(s->nq_stat), now, last_time, size_queue(s->queue));

    //Switch para os possiveis eventos
    //Tratamento dos eventos descrito no relatorio
    switch ( e.type ) {
        case EVENT_arrival: {
            assert( now == p.arrived_time );
            const Time t_arr = now + randExp(s->rand, s->lambda);
            const Event e_arr = create_event_arrival(t_arr, s->color);
            assert( e_arr.time == e_arr.person.arrived_time );
            insert_heap(s->events, e_arr);
            if ( s->busy ) {
                insert_queue(s->queue, p);
            } else {
                record_wait_time(&(s->wt_stat), s->color, now, p);
                const Time t_lea = now + randExp(s->rand, s->mu);
                const Event e_lea = create_event_leave(t_lea, p);
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
                record_wait_time(&(s->wt_stat), s->color, now, new_p_lea);
                const Time new_t_lea = now + randExp(s->rand, s->mu);
                const Event new_e_lea = create_event_leave(new_t_lea, new_p_lea);
                insert_heap(s->events, new_e_lea);
            }
        } break;
        default: {
            assert( 0 && "Unreachable" );
        } break;
    }
    s->curr_time = now;
    return e;
}

#ifdef SYSTEM_MAIN
#undef SYSTEM_MAIN

#include "test.c"

const char *event_type_name[] = { "arrival", "leave", };


//Funcao para printar os eventos e a fila de espera
void log_system(const System *s) {
    log("\nEvents:");
    {
        const u32 esize = s->events->size;
        for ( u32 i = 0; i < esize; i++ ) {
            const Event e = s->events->events[i];
            log(" (%s - %6.4lf | %u - %6.4lf)",
                    event_type_name[e.type], e.time,
                    e.person.color, e.person.arrived_time);
        }
    }
    log("\nQueue:");
    {
        const Queue *q = s->queue;
        const u32 qsize = size_queue(q);
        for ( u32 i = 0; i < qsize; i++ ) {
            const u32 index = (q->head + i + q->len) % q->len;
            const Person p = q->people[index];
            if ( i % 4 == 0 ) {
                log("\n");
            }
            log(" (%u | %6.4lf)",
                    p.color, p.arrived_time);
        }
    }
    log("\n");
}

//Funcao de teste do sistema
void test_system(System *s, const u32 leaves_to_count) {
    f64 last_time = s->curr_time;
    b32 last_busy = s->busy;
    u32 last_qsize = size_queue(s->queue);
    u32 last_esize = size_heap(s->events);
    for ( u32 i = 0; s->wt_stat.n < leaves_to_count; i++ ) {
        log("Before event %u\n", i);
        const Event e = handle_next_event(s);
        const f64 curr_time  = s->curr_time;
        const u32 curr_busy  = s->busy;
        const u32 curr_qsize = size_queue(s->queue);
        const u32 curr_esize = size_heap(s->events);
        log("Handled event: %s (%6.4lf)\n",
                event_type_name[e.type], e.time);
        log("Person: arrived (%6.4lf) color (%u)\n",
                e.person.arrived_time, e.person.color);
        log("Curr time    : %6.4lf -> %6.4lf\n",
                last_time, curr_time);
        log("Busy         : %u -> %u\n",
                last_busy, curr_busy);
        log("Queue size   : %u -> %u\n",
                last_qsize, curr_qsize);
        log("Events size  : %u -> %u\n",
                last_esize, curr_esize);
        log("\n");

        f64_expect_less_eq("System curr time advances",
                last_time, curr_time);
        switch ( e.type ) {
            case EVENT_arrival: {
                u32_expect_equal(
                        "System is busy after arrival",
                        1, curr_busy);
                if ( last_busy ) {
                    u32_expect_equal(
                            "System Queue grows after busy arrival",
                            last_qsize + 1, curr_qsize);
                } else {
                    u32_expect_equal(
                            "System Queue is empty after idle arrival",
                            0, curr_qsize);
                }
                u32_expect_equal(
                        "System Events size is 2 after any arrival",
                        2, curr_esize);
            } break;
            case EVENT_leave: {
                if ( last_qsize > 0 ) {
                    u32_expect_equal(
                            "System is busy after non-empty leave",
                            1, curr_busy);
                    u32_expect_equal(
                            "System Queue shrinks after non-empty leave",
                            last_qsize - 1, curr_qsize);
                    u32_expect_equal(
                            "System Events size is 2 after non-empty leave",
                            2, curr_esize);
                } else {
                    u32_expect_equal(
                            "System is not busy after empty leave",
                            0, curr_busy);
                    u32_expect_equal(
                            "System Queue is empty after empty leave",
                            0, curr_qsize);
                    u32_expect_equal(
                            "System Events size is 1 after empty leave",
                            1, curr_esize);
                }
            } break;
            default: {
                assert( 0 && "Unreachable" );
            } break;
        }

        last_time  = curr_time;
        last_busy  = curr_busy;
        last_qsize = curr_qsize;
        last_esize = curr_esize;
    }
}

//Teste da proxima rodada
void test_next_batch(System *s) {
    const System snapshot = *s;
    next_batch(s);
    f64_expect_equal_tol("next_batch keeps lambda",
            snapshot.lambda, s->lambda, 0.0);
    f64_expect_equal_tol("next_batch keeps mu",
            snapshot.mu, s->mu, 0.0);
    u32_expect_equal("next_batch keeps busy",
            snapshot.busy, s->busy);
    u32_expect_equal("next_batch increments color",
            snapshot.color + 1, s->color);
    f64_expect_equal_tol("next_batch resets curr_time",
            0.0, s->curr_time, 0.0);
    u32_expect_equal("next_batch resets nq_stat.n",
            0, s->nq_stat.n);
    f64_expect_equal_tol("next_batch resets nq_stat.acc",
            0.0, s->nq_stat.acc, 0.0);
    f64_expect_equal_tol("next_batch resets nq_stat.sqr_acc",
            0.0, s->nq_stat.sqr_acc, 0.0);
    u32_expect_equal("next_batch resets wt_stat.n",
            0, s->wt_stat.n);
    f64_expect_equal_tol("next_batch resets wt_stat.acc",
            0.0, s->wt_stat.acc, 0.0);
    f64_expect_equal_tol("next_batch resets wt_stat.sqr_acc",
            0.0, s->wt_stat.sqr_acc, 0.0);
}

//Funcao para printar as estatisticas do sistema
void check_stats(const System *s,
        const f64 expected_wt_avg, const f64 expected_wt_var,
        const f64 expected_qs_avg, const f64 expected_qs_var) {
    const f64
        wt_avg = discrete_average(s->wt_stat),
        wt_var = discrete_variance(s->wt_stat),
        qs_avg = continuous_average(s->nq_stat, s->curr_time),
        qs_var = continuous_variance(s->nq_stat, s->curr_time);
    log("Wait time (%u samples):\n", s->wt_stat.n);
    log("    avg: %6.4lf,     var: %6.4lf\n",
            wt_avg, wt_var);
    log("exp_avg: %6.4lf, exp_var: %6.4lf\n",
            expected_wt_avg, expected_wt_var);
    f64_expect_equal("System wait time average (2*lambda = mu)",
            expected_wt_avg, wt_avg);
    f64_expect_equal("System wait time variance (2*lambda = mu)",
            expected_wt_var, wt_var);
    log("Queue size:\n");
    log("    avg: %6.4lf,     var: %6.4lf\n",
            qs_avg, qs_var);
    log("exp_avg: %6.4lf, exp_var: %6.4lf\n",
            expected_qs_avg, expected_qs_var);
    f64_expect_equal("System queue size average (2*lambda = mu)",
            expected_qs_avg, qs_avg);
    f64_expect_equal("System queue size variance (2*lambda = mu)",
            expected_qs_var, qs_var);
}

//Funcao para rodar os testes do sistema
int main() {
    SECTION("System 2*lambda = mu (randExp = 1) test");

    f64 table[1] = { exp(-1.0) };
    RandTable rand_table = create_table_ctx(table, 1);
    const f64 lambda = 0.5;
    const f64 mu = 2.0 * lambda;
    RandCtx *ctx = (RandCtx *) &rand_table;
    EventHeap e_heap = init_heap(), *eh = &e_heap;
    Queue queue = init_queue(Queue_FCFS), *q = &queue;
    const Color init_color = 0;
    System sys = init_system(ctx, lambda, mu, eh, q, init_color),
           *s = &sys;

    u32_expect_equal("Just initialized system is empty",
            1, is_empty_system(s));
    add_first_event(s);
    u32_expect_equal("System with first event is not empty",
            0, is_empty_system(s));

    const u32 leaves_to_count = 5;

    test_system(s, leaves_to_count);
    check_stats(s, 0.0, 0.0, 0.0, 0.0);
    log_system(s);

    end_tests("System 2*lambda = mu");

    SECTION("System lambda = mu (randExp = 1) test");
    test_next_batch(s);

    s->lambda = mu;

    test_system(s, leaves_to_count);
    check_stats(s, 0.0, 0.0, 0.0, 0.0);
    log_system(s);

    end_tests("System lambda = mu");

    SECTION("System lambda = 2*mu (randExp = 1) test");
    test_next_batch(s);

    s->lambda = 2.0 * mu;

    test_system(s, 2 * leaves_to_count);
    check_stats(s, 2.25, 2.2916666666666667, 4.05, 8.0475);
    log_system(s);

    end_tests("System lambda = 2*mu");

    SECTION("System 2*lambda = mu (randExp = 1) test 2");
    test_next_batch(s);

    s->lambda = lambda;

    test_system(s, leaves_to_count);
    check_stats(s, 7.0, 2.5, 5.769230769230769, 3.5621301775147954);
    log_system(s);

    end_tests("System 2*lambda = mu (2)");

    deinit_system(s);
}

#endif // SYSTEM_MAIN

#endif // SYSTEM_IMPL
