#ifndef STATS_HEADER
#define STATS_HEADER
#include "types.h"

typedef struct _Stats  {
    u32 n;
    f64 acc;
    f64 sqr_acc;
} Stats;

Stats new_stats();
Stats accumulate(Stats stat, f64 val);
void acc_and_update(Stats *stat, f64 val);
f64 average(Stats stat);
f64 variance(Stats stat);

#endif // STATS_HEADER

#ifdef STATS_IMPL
#undef STATS_IMPL
#include <assert.h>

Stats new_stats() {
    const Stats ret = { 0 };
    return ret;
}

Stats accumulate(Stats stat, f64 val) {
    const Stats ret = {
        .n = stat.n + 1,
        .acc = stat.acc + val,
        .sqr_acc = stat.sqr_acc + val * val,
    };
    return ret;
}

void acc_and_update(Stats *stat, f64 val) {
    *stat = accumulate(*stat, val);
}

f64 average(Stats stat) {
    assert( stat.n > 0 );
    return stat.acc / ((f64) stat.n);
}

f64 variance(Stats stat) {
    assert( stat.n > 1 );
    return (stat.sqr_acc - (stat.acc * stat.acc / ((f64) stat.n))) / ((f64) (stat.n - 1));
}

#endif // STATS_IMPL
