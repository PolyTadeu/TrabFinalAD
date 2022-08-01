#ifndef STATS_HEADER
#define STATS_HEADER
#include "types.h"

typedef struct _Stats  {
    u32 n;
    f64 acc;
    f64 sqr_acc;
} Stats;

typedef struct _CI {
    f64 up;
    f64 low;
    f64 precision;
} CI;

typedef struct _CachedStats {
    f64 avg;
    f64 var;
    CI ci_avg;
    CI tstudent;
    CI chi;
} CachedStats;

Stats new_stats();
Stats accumulate(Stats stat, f64 val, f64 mul);
void acc_and_update(Stats *stat, f64 val, f64 mul);
f64 discrete_average(Stats stat);
f64 discrete_variance(Stats stat);
f64 continuous_average(Stats stat, f64 t);
f64 continuous_variance(Stats stat, f64 t);

CachedStats cache_stats(const Stats stat);

#endif // STATS_HEADER

#ifdef STATS_IMPL
#ifndef STATS_IMPL_ED
#undef STATS_IMPL
#define STATS_IMPL_ED
#include <assert.h>
#include <math.h>

Stats new_stats() {
    const Stats ret = {
        .n = 0,
        .acc = 0.0,
        .sqr_acc = 0.0,
    };
    return ret;
}

Stats accumulate(Stats stat, f64 val, f64 mul) {
    const Stats ret = {
        .n = stat.n + 1,
        .acc = stat.acc + (val * mul),
        .sqr_acc = stat.sqr_acc + (val * val * mul),
    };
    return ret;
}

void acc_and_update(Stats *stat, f64 val, f64 mul) {
    *stat = accumulate(*stat, val, mul);
}

f64 discrete_average(Stats stat) {
    assert( stat.n > 0 );
    return stat.acc / ((f64) stat.n);
}

f64 discrete_variance(Stats stat) {
    assert( stat.n > 1 );
    return (stat.sqr_acc - (stat.acc * stat.acc / ((f64) stat.n))) / ((f64) (stat.n - 1));
}

f64 continuous_average(Stats stat, f64 t) {
    return stat.acc / t;
}

f64 continuous_variance(Stats stat, f64 t) {
    const f64 avg = continuous_average(stat, t);
    const f64 sqr_avg = avg * avg;
    return (stat.sqr_acc - sqr_avg) / t;
}

#define TCONST      1.645
#define CHI_HIGH    0.025
#define CHI_LOW     0.975

CachedStats cache_stats(const Stats stat) {
    assert( stat.n >= 30 );
    const f64 avg = discrete_average(stat);
    const f64 var = discrete_variance(stat);
    const f64 diff_t = TCONST * sqrt(var / ((f64) stat.n));
    const f64 chi_const = ((f64) (stat.n - 1)) * var;
    CachedStats ret = {
        .avg = avg,
        .var = var,
        .ci_avg = {
            .up = avg + diff_t,
            .low = avg - diff_t,
            .precision = diff_t / avg,
        },
        .tstudent = {
            .up = avg + diff_t,
            .low = avg - diff_t,
            .precision = diff_t / avg,
        },
        .chi = {
            .up = chi_const / CHI_HIGH,
            .low = chi_const / CHI_LOW,
            .precision = (CHI_LOW - CHI_HIGH) / (CHI_LOW + CHI_HIGH),
        },
    };
    return ret;
}

#endif // STATS_IMPL_ED

#endif // STATS_IMPL
