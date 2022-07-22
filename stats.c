#ifndef STATS_HEADER
#define STATS_HEADER

typedef struct _Stats  {
    unsigned int n;
    double acc;
    double sqr_acc;
} Stats;

Stats new_stats();
Stats accumulate(Stats stat, double val);
void acc_and_update(Stats *stat, double val);
double average(Stats stat);
double variance(Stats stat);

#endif // STATS_HEADER

#ifdef STATS_IMPL
#undef STATS_IMPL
#include <assert.h>

Stats new_stats() {
    const Stats ret = { 0 };
    return ret;
}

Stats accumulate(Stats stat, double val) {
    const Stats ret = {
        .n = stat.n + 1,
        .acc = stat.acc + val,
        .sqr_acc = stat.sqr_acc + val * val,
    };
    return ret;
}

void acc_and_update(Stats *stat, double val) {
    *stat = accumulate(*stat, val);
}

double average(Stats stat) {
    assert( stat.n > 0 );
    return stat.acc / ((double) stat.n);
}

double variance(Stats stat) {
    assert( stat.n > 1 );
    return (stat.sqr_acc - (stat.acc * stat.acc / ((double) stat.n))) / ((double) (stat.n - 1));
}

#endif // STATS_IMPL