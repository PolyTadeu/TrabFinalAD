#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "args.c"

#define STATS_IMPL
#include "stats.c"
#define SYSTEM_IMPL
#include "system.c"

// Argumentos
ARGS_ArgOption options[] = {
    [0] = {
        .small = "s",
        .big = "seed",
        .description = "Chooses the initial seed,"
            " (default: current time)",
        .type = ARGS_uint,
    },
    [1] = {
        .small = "k",
        .big = "round-size",
        .description = "Chooses the size of the round (K),"
            " (default: 100)",
        .type = ARGS_uint,
    },
    [2] = {
        .small = "l",
        .big = "lambda",
        .description = "Chooses the rate of arrival (lambda),"
            " (default: 0.5)",
        .type = ARGS_float,
    },
    [3] = {
        .small = "m",
        .big = "mu",
        .description = "Chooses the rate of service (mu),"
            " (default: 1)",
        .type = ARGS_float,
    },
    [4] = {
        .small = "n",
        .big = "round-count",
        .description = "Chooses the number of rounds (N),"
            " (default: 3200)"
            "\nNote: the CI calculation is"
            " hardcoded to work with 3200",
        .type = ARGS_uint,
    },
    [5] = {
        .small = "t",
        .big = "transient-size",
        .description = "Chooses the number of events to be handled"
            " before finishing the transient fase,"
            " (default: 1000000)",
        .type = ARGS_uint,
    },
    [6] = {
        .small = "",
        .big = "lcfs",
        .description = "Sets the queue behavior to LCFS,"
            " (default: FCFS)",
        .type = ARGS_flag,
    },
    [7] = {
        .small = "v",
        .big = "verbose",
        .description = "Tells the program to print statistics"
            " after each round",
        .type = ARGS_flag,
    },
};

const ARGS_ArgSpec spec = {
    .flags = 0,
    .options_len = sizeof(options)/sizeof(options[0]),
    .options = options,
};

// Estrutura para options
typedef struct {
    u64 seed;
    u64 round_size;
    f64 lambda;
    f64 mu;
    u64 round_count;
    u64 transient_size;
    u8 lcfs;
    u8 verbose;
    u8 ARGS_valid;
} Options;

// Funcao para printar a struct Options no stdout
int gen_struct() {
    ARGS_struct_from_spec(spec);
    return 0;
}

// Funcao de print das options
void print_opts(const Options opts) {
    const char *qtype_name[] = { "FCFS", "LCFS", };
    printf("Seed       : %lu\n", opts.seed);
    printf("Round size : %lu\n", opts.round_size);
    printf("Round count: %lu\n", opts.round_count);
    printf("Lambda     : %lf\n", opts.lambda);
    printf("Mu         : %lf\n", opts.mu);
    printf("Transi size: %lu\n", opts.transient_size);
    printf("Queue type : %s\n\n", qtype_name[opts.lcfs]);
}

// Funcao para printar as estatisticas
void print_stats(const char *name,
        const CachedStats mu, const CachedStats sigma) {
    printf("Avg %s: %lf\n", name, mu.avg);
    printf("Var %s: %lf\n\n", name, sigma.avg);

    printf("IC Avg %s : [%lf - %lf] (precision %lf)\n",
            name, mu.tstudent.low, mu.tstudent.up,
            mu.tstudent.precision);

    printf("IC Var t-Student %s   : [%lf - %lf] (prec %lf)\n",
            name, sigma.tstudent.low, sigma.tstudent.up,
            sigma.tstudent.precision);

    printf("IC Var Chi-squared %s : [%lf - %lf] (prec %lf)\n\n",
            name, sigma.chi.low, sigma.chi.up,
            sigma.chi.precision);
}

// Main para rodar o codigo
int main(const int argc, const char **argv) {
    Options opts = {
        .seed = time(NULL),
        .round_size = 100,
        .lambda = 0.5,
        .mu = 1,
        .round_count = 3200,
        .transient_size = 1000000,
        .lcfs = 0,
        .verbose = 0,
    };
    ARGS_parse(spec, (void*) &opts, sizeof(opts), argc, argv);
    if ( !opts.ARGS_valid ) {
        return 0;
    }

    // Inicializar sistema
    RandCtx rand = create_rand_ctx(opts.seed);
    EventHeap e_heap = init_heap(), *eh = &e_heap;
    const QueueType qtype = opts.lcfs ? Queue_LCFS : Queue_FCFS;
    Queue queue = init_queue(qtype), *q = &queue;
    const Color init_color = 0;
    System sys = init_system(&rand, opts.lambda, opts.mu,
            eh, q, init_color),
           *s = &sys;
    Stats wt_mu_hat = new_stats(),
          wt_sigma_hat = new_stats(),
          nq_mu_hat = new_stats(),
          nq_sigma_hat = new_stats();
    add_first_event(s);

    clock_t clock1 = clock(), clock2 = clock1;
    // Fase Transiente
    for ( ; s->wt_stat.n < opts.transient_size; ) {
        handle_next_event(s);
    }
    if ( opts.verbose ) {
        clock2 = clock() - clock2;
        printf("transient: %lf seconds (clock)\n",
                ((f64) clock2) / ((f64) CLOCKS_PER_SEC));
        clock2 = clock();
    }

    // Rodadas
    for ( u64 i = 0; i < opts.round_count; i++ ) {
        next_batch(s);
        for ( ; s->wt_stat.n < opts.round_size; ) {
            handle_next_event(s);
        }
        const f64
            wt_avg_i = discrete_average(s->wt_stat),
            wt_var_i = discrete_variance(s->wt_stat),
            nq_avg_i = continuous_average(s->nq_stat, s->curr_time),
            nq_var_i = continuous_variance(s->nq_stat, s->curr_time);
        acc_and_update(&wt_mu_hat, wt_avg_i, 1);
        acc_and_update(&wt_sigma_hat, wt_var_i, 1);
        acc_and_update(&nq_mu_hat, nq_avg_i, 1);
        acc_and_update(&nq_sigma_hat, nq_var_i, 1);
        if ( opts.verbose ) {
            clock2 = clock() - clock2;
            printf("round %4lu: %lf seconds (so far %lf)\n",
                    i, ((f64) clock2) / ((f64) CLOCKS_PER_SEC),
                    ((f64)(clock()-clock1)) / ((f64) CLOCKS_PER_SEC));
            printf("Wait time : avg %lf, var %lf\n",
                    wt_avg_i, wt_var_i);
            printf("(So far)    average %lf, variance %lf\n",
                discrete_average(wt_mu_hat),
                discrete_average(wt_sigma_hat));
            printf("Queue size: average %lf, variance %lf\n",
                    nq_avg_i, nq_var_i);
            printf("(So far)    average %lf, variance %lf\n",
                discrete_average(nq_mu_hat),
                discrete_average(nq_sigma_hat));
            printf("\n");
            clock2 = clock();
        }
    }
    assert( wt_mu_hat.n == opts.round_count );
    assert( wt_sigma_hat.n == opts.round_count );
    assert( nq_mu_hat.n == opts.round_count );
    assert( nq_sigma_hat.n == opts.round_count );

    clock1 = clock() - clock1;

    const CachedStats cache_wt_mu = cache_stats(wt_mu_hat),
          cache_wt_sigma = cache_stats(wt_sigma_hat),
          cache_nq_mu = cache_stats(nq_mu_hat),
          cache_nq_sigma = cache_stats(nq_sigma_hat);

    print_opts(opts);

    printf("Total runtime : %lf seconds (clock)\n\n",
            ((f64) clock1) / ((f64) CLOCKS_PER_SEC));

    print_stats("Wait time", cache_wt_mu, cache_wt_sigma);
    print_stats("Queue size", cache_nq_mu, cache_nq_sigma);

    return 0;
}
