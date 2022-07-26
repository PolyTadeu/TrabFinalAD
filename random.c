#ifndef RAND_HEADER
#define RAND_HEADER

typedef enum _RandType {
    RandType_RANDC,
} RandType;

// contexto necessário para gerar uniforme
typedef struct _RandCtx {
    double (*uniform)(struct _RandCtx *);
    RandType type;
} RandCtx;

RandCtx create_rand_ctx(unsigned int);
double randUniform(RandCtx *ctx);
double randExp(RandCtx *ctx, double lambda);

#endif // RAND_HEADER

#ifdef RAND_IMPL
#undef RAND_IMPL
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// gera um uniforme a partir da biblioteca de C (rand)
double rand_uni(RandCtx *ctx) {
    assert( ctx->type == RandType_RANDC );
    return ((double) rand()) / ((double) RAND_MAX);
}

// cria um contexto de random
RandCtx create_rand_ctx(unsigned int t) {
    srand(t);
    RandCtx ret = {
        .uniform = rand_uni,
        .type = RandType_RANDC,
    };
    return ret;
}

// função que uniforme apartir de qualquer RandCtx
double randUniform(RandCtx *ctx) {
    return ctx->uniform(ctx);
}

// função que uniforme apartir de qualquer RandCtx
double randExp(RandCtx *ctx, double lambda) {
    double val = randUniform(ctx);
    return -1.0f * (log(val) / lambda);
}

#ifdef RAND_MAIN
#undef RAND_MAIN
#include <stdio.h>
#include <time.h>

#define STATS_IMPL
#include "stats.c"

int main() {
    RandCtx ctx = create_rand_ctx((unsigned int) time(NULL));
    const int n = 100000;
    Stats stat = new_stats();

    for ( unsigned int i = 0; i < n; i++ ) {
        const double val = randUniform(&ctx);
        acc_and_update(&stat, val);
    }

    const double mediaUni = average(stat), varUni = variance(stat);
    const double unimedia = 0.5f, univar = 1.0f / 12.0f;
    printf("   media: %7.7lf,    var: %7.7lf\n", mediaUni, varUni);
    printf("unimedia: %7.7lf, univar: %7.7lf\n\n", unimedia, univar);

    stat = new_stats();
    const double lambda = 7.3f;
    for ( unsigned int i = 0; i < n; i++ ) {
        const double val = randExp(&ctx, lambda);
        acc_and_update(&stat, val);
    }

    const double media = average(stat), var = variance(stat);
    const double expmedia = 1 / lambda, expvar = expmedia * expmedia;
    printf("   media: %7.7lf,    var: %7.7lf\n", media, var);
    printf("expmedia: %7.7lf, expvar: %7.7lf\n\n", expmedia, expvar);

}
#endif // RAND_MAIN
#endif // RAND_IMPL
