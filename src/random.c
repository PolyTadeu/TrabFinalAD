#ifndef RAND_HEADER
#define RAND_HEADER
#include "types.h"

typedef enum _RandType {
    RandType_RANDC,
} RandType;

// contexto necessário para gerar uniforme
typedef struct _RandCtx {
    f64 (*uniform)(struct _RandCtx *);
    RandType type;
} RandCtx;

RandCtx create_rand_ctx(u32);
f64 randUniform(RandCtx *ctx);
f64 randExp(RandCtx *ctx, f64 lambda);

#endif // RAND_HEADER

#ifdef RAND_IMPL
#undef RAND_IMPL
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// gera um uniforme a partir da biblioteca de C (rand)
f64 rand_uni(RandCtx *ctx) {
    assert( ctx->type == RandType_RANDC );
    return ((f64) rand()) / ((f64) RAND_MAX);
}

// cria um contexto de random
RandCtx create_rand_ctx(u32 t) {
    srand(t);
    RandCtx ret = {
        .uniform = rand_uni,
        .type = RandType_RANDC,
    };
    return ret;
}

// função que uniforme apartir de qualquer RandCtx
f64 randUniform(RandCtx *ctx) {
    return ctx->uniform(ctx);
}

// função que uniforme apartir de qualquer RandCtx
f64 randExp(RandCtx *ctx, f64 lambda) {
    f64 val = randUniform(ctx);
    return -1.0f * (log(val) / lambda);
}

#ifdef RAND_MAIN
#undef RAND_MAIN
#include <stdio.h>
#include <time.h>

#define STATS_IMPL
#include "stats.c"

int main() {
    RandCtx ctx = create_rand_ctx((u32) time(NULL));
    const u32 n = 100000;
    Stats stat = new_stats();

    for ( u32 i = 0; i < n; i++ ) {
        const f64 val = randUniform(&ctx);
        acc_and_update(&stat, val);
    }

    const f64 mediaUni = average(stat), varUni = variance(stat);
    const f64 unimedia = 0.5f, univar = 1.0f / 12.0f;
    printf("   media: %7.7lf,    var: %7.7lf\n", mediaUni, varUni);
    printf("unimedia: %7.7lf, univar: %7.7lf\n\n", unimedia, univar);

    stat = new_stats();
    const f64 lambda = 7.3f;
    for ( u32 i = 0; i < n; i++ ) {
        const f64 val = randExp(&ctx, lambda);
        acc_and_update(&stat, val);
    }

    const f64 media = average(stat), var = variance(stat);
    const f64 expmedia = 1 / lambda, expvar = expmedia * expmedia;
    printf("   media: %7.7lf,    var: %7.7lf\n", media, var);
    printf("expmedia: %7.7lf, expvar: %7.7lf\n\n", expmedia, expvar);

}
#endif // RAND_MAIN
#endif // RAND_IMPL
