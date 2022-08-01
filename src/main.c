#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "types.h"
#include "args.c"

#define STATS_IMPL
#include "stats.c"
#define SYSTEM_IMPL
#include "system.c"

ARGS_ArgOption options[] = {
    [0] = {
        .small = "s",
        .big = "seed",
        .type = ARGS_uint,
    },
    [1] = {
        .small = "k",
        .big = "round-size",
        .type = ARGS_uint,
    },
    [2] = {
        .small = "l",
        .big = "lambda",
        .type = ARGS_float,
    },
    [3] = {
        .small = "m",
        .big = "mu",
        .type = ARGS_float,
    },
    [4] = {
        .small = "n",
        .big = "round-count",
        .type = ARGS_uint,
    },
    [5] = {
        .small = "",
        .big = "lcfs",
        .type = ARGS_flag,
    },
    [6] = {
        .small = "v",
        .big = "verbose",
        .type = ARGS_flag,
    },
};

const ARGS_ArgSpec spec = {
    .flags = 0,
    .options_len = sizeof(options)/sizeof(options[0]),
    .options = options,
};

typedef struct {
    u64 seed;
    u64 round_size;
    f64 lambda;
    f64 mu;
    u64 round_count;
    u8 lcfs;
    u8 verbose;
    u8 ARGS_valid;
} Options;

int gen_struct() {
    ARGS_struct_from_spec(spec);
    return 0;
}

int main(const int argc, const char **argv) {
    Options opts = {
        .seed = time(NULL),
        .round_size = 100,
        .lambda = 0.5,
        .mu = 1,
        .round_count = 3200,
        .lcfs = 0,
        .verbose = 0,
    };
    ARGS_parse(spec, (void*) &opts, sizeof(opts), argc, argv);
    if ( !opts.ARGS_valid ) {
        return 0;
    }

    printf("opts = {\n");
    printf("    .seed = %lu\n", opts.seed);
    printf("    .round_size = %lu\n", opts.round_size);
    printf("    .lambda = %lf\n", opts.lambda);
    printf("    .mu = %lf\n", opts.mu);
    printf("    .round_count = %lu\n", opts.round_count);
    printf("    .lcfs = %hhu\n", opts.lcfs);
    printf("    .verbose = %hhu\n", opts.verbose);
    printf("    .ARGS_valid = %hhu\n", opts.ARGS_valid);
    printf("}\n");

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
    for ( ; s->wt_stat.n < 1000000; ) {
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
        const f64 wt_avg_i = discrete_average(s->wt_stat),
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

    const char *qtype_name[] = { "FCFS", "LCFS", };
    printf("Total runtime : %lf seconds (clock)\n",
            ((f64) clock1) / ((f64) CLOCKS_PER_SEC));
    printf("Queue type : %s\n", qtype_name[s->queue->type]);
    printf("Avg Wait time : %lf\n", cache_wt_mu.avg);
    printf("Var Wait time : %lf\n\n", cache_wt_sigma.avg);

    printf("IC Avg Wait time : [%lf - %lf] (precision %lf)\n",
            cache_wt_mu.tstudent.low, cache_wt_mu.tstudent.up,
            cache_wt_mu.tstudent.precision);

    printf("IC Var t-Student Wait time   : [%lf - %lf] (prec %lf)\n",
            cache_wt_sigma.tstudent.low, cache_wt_sigma.tstudent.up,
            cache_wt_sigma.tstudent.precision);

    printf("IC Var Chi-squared Wait time : [%lf - %lf] (prec %lf)\n\n",
            cache_wt_sigma.chi.low, cache_wt_sigma.chi.up,
            cache_wt_sigma.chi.precision);

    printf("Avg Queue size: %lf\n", cache_nq_mu.avg);
    printf("Var Queue size: %lf\n\n", cache_nq_sigma.avg);

    printf("IC Avg Queue size : [%lf - %lf] (precision %lf)\n",
            cache_nq_mu.tstudent.low, cache_nq_mu.tstudent.up,
            cache_nq_mu.tstudent.precision);

    printf("IC Var t-Student Queue size   : [%lf - %lf] (prec %lf)\n",
            cache_nq_sigma.tstudent.low, cache_nq_sigma.tstudent.up,
            cache_nq_sigma.tstudent.precision);

    printf("IC Var Chi-squared Queue size : [%lf - %lf] (prec %lf)\n\n",
            cache_nq_sigma.chi.low, cache_nq_sigma.chi.up,
            cache_nq_sigma.chi.precision);

    return 0;
}
