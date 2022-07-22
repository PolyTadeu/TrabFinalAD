#ifndef RAND_HEADER
#define RAND_HEADER

typedef enum _RandType {
    RandType_RANDC,
} RandType;

// gerar uniforme
typedef struct _RandCtx {
    double (*uniform)(struct _RandCtx *);
    RandType type;
} RandCtx;

RandCtx create_rand_ctx(unsigned int);
double randUniform(RandCtx *ctx);
double randExp(RandCtx *ctx, double lambda);

#endif // RAND_HEADER

#ifdef RAND_IMPL
#include <stdlib.h>
#include <math.h>
#include <assert.h>

double rand_uni(RandCtx *ctx) {
    assert( ctx->type == RandType_RANDC );
    return ((double) rand()) / ((double) RAND_MAX);
}

RandCtx create_rand_ctx(unsigned int t) {
    srand(t);
    RandCtx ret = {
        .uniform = rand_uni,
        .type = RandType_RANDC,
    };
    return ret;
}

double randUniform(RandCtx *ctx) {
    return ctx->uniform(ctx);
}

double randExp(RandCtx *ctx, double lambda) {
    double val = randUniform(ctx);
    return -1.0f * (log(val) / lambda);
}

#ifdef RAND_MAIN
#include <stdio.h>
#include <time.h>

int main() {
    RandCtx ctx = create_rand_ctx((unsigned int) time(NULL));
    double soma = 0.0f, somasqr = 0.0f;
    const int n = 100000;
    const int nf = (double) n;

    for ( unsigned int i = 0; i < n; i++ ) {
        const double val = randUniform(&ctx);
        soma += val;
        somasqr += val * val;
    }

    const double mediaUni = soma / nf, var1Uni = (somasqr - (soma*soma/nf)) / (nf-1),
        var2Uni = (somasqr / (nf-1)) - ((soma*soma/nf) / (nf-1));
    const double unimedia = 0.5f, univar = 1.0f / 12.0f;
    printf("   media: %7.7lf,   var1: %7.7lf, var2: %7.7lf\n", mediaUni, var1Uni, var2Uni);
    printf("unimedia: %7.7lf, univar: %7.7lf\n\n", unimedia, univar);

    soma = somasqr = 0;
    const double lambda = 7.3f;
    for ( unsigned int i = 0; i < n; i++ ) {
        const double val = randExp(&ctx, lambda);
        soma += val;
        somasqr += val * val;
    }

    const double media = soma / nf, var1 = (somasqr - (soma*soma/nf)) / (nf-1),
        var2 = (somasqr / (nf-1)) - ((soma*soma/nf) / (nf-1));
    const double expmedia = 1 / lambda, expvar = expmedia * expmedia;
    printf("   media: %7.7lf,   var1: %7.7lf, var2: %7.7lf\n", media, var1, var2);
    printf("expmedia: %7.7lf, expvar: %7.7lf\n\n", expmedia, expvar);

}
#endif // RAND_MAIN
#endif // RAND_IMPL