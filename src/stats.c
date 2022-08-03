#ifndef STATS_HEADER
#define STATS_HEADER
#include "types.h"

//Estrutura de status de amostrar e incrementadores
typedef struct _Stats  {
    u32 n;
    f64 acc;
    f64 sqr_acc;
} Stats;

//Estrutura do intervalo de confianca
typedef struct _CI {
    f64 up;
    f64 low;
    f64 precision;
} CI;

//Estrutura para salvar as estatisticas
typedef struct _CachedStats {
    f64 avg;
    f64 var;
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

// Inicia estrutuurua de status com valores nulos
Stats new_stats() {
    const Stats ret = {
        .n = 0,
        .acc = 0.0,
        .sqr_acc = 0.0,
    };
    return ret;
}

//Aumentar os valores das amostras e dos incrementadores
Stats accumulate(Stats stat, f64 val, f64 mul) {
    const Stats ret = {
        .n = stat.n + 1,
        .acc = stat.acc + (val * mul),
        .sqr_acc = stat.sqr_acc + (val * val * mul),
    };
    return ret;
}

//Funcao para acumular e atualizar
//Utilzia a funcao descrita anteriormente
void acc_and_update(Stats *stat, f64 val, f64 mul) {
    *stat = accumulate(*stat, val, mul);
}

//Funcao para calcular media em tempo discreta
f64 discrete_average(Stats stat) {
    assert( stat.n > 0 );
    return stat.acc / ((f64) stat.n);
}

///Funcao para calcular varianca em tempo discreto
f64 discrete_variance(Stats stat) {
    assert( stat.n > 1 );
    return (stat.sqr_acc - (stat.acc * stat.acc / ((f64) stat.n))) / ((f64) (stat.n - 1));
}

//Funcao para calcular a media em tempo continuo
f64 continuous_average(Stats stat, f64 t) {
    return stat.acc / t;
}

//Funcao para calcuula a varianca em tempo continuo
f64 continuous_variance(Stats stat, f64 t) {
    const f64 avg = continuous_average(stat, t);
    const f64 sqr_avg = avg * avg;
    return (stat.sqr_acc / t) - sqr_avg;
}

#define TCONST      1.960
#define CHI_HIGH    3069.559
#define CHI_LOW     3332.715


//Funcao para salvar as estatisticas na estrutura de cached status
CachedStats cache_stats(const Stats stat) {
    assert( stat.n >= 30 );
    const f64 avg = discrete_average(stat);
    const f64 var = discrete_variance(stat);
    const f64 diff_t = TCONST * sqrt(var / ((f64) stat.n));
    const f64 chi_const = ((f64) (stat.n - 1)) * avg;
    CachedStats ret = {
        .avg = avg,
        .var = var,
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
