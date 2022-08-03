#ifndef RAND_HEADER
#define RAND_HEADER
#include "types.h"

typedef enum _RandType {
    RandType_RANDC,
    RandType_Table,
} RandType;

// Contexto necessário para gerar uniforme
typedef struct _RandCtx {
    f64 (*uniform)(struct _RandCtx *);
    RandType type;
} RandCtx;

// Contexto necessário para gerar uniforme
typedef struct _RandTable {
    RandCtx ctx;
    f64 *table;
    u32 len;
    u32 next;
} RandTable;

RandCtx create_rand_ctx(u32 t);
RandTable create_table_ctx(f64 *table, u32 len);
f64 randUniform(RandCtx *ctx);
f64 randExp(RandCtx *ctx, f64 lambda);

#endif // RAND_HEADER

#ifdef RAND_IMPL
#undef RAND_IMPL
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// Gera um uniforme a partir da biblioteca de C (rand)
f64 rand_uni(RandCtx *ctx) {
    assert( ctx->uniform == rand_uni );
    assert( ctx->type == RandType_RANDC );
    return ((f64) rand()) / ((f64) RAND_MAX);
}

// Cria um contexto de random do c
RandCtx create_rand_ctx(u32 t) {
    srand(t);
    RandCtx ret = {
        .uniform = rand_uni,
        .type = RandType_RANDC,
    };
    return ret;
}

// Retorna proximo valor aleatorio da tabela
f64 table_uni(RandCtx *ctx) {
    assert( ctx->uniform == table_uni );
    assert( ctx->type == RandType_Table );
    RandTable *tctx = (RandTable *) ctx;
    f64 ret = tctx->table[tctx->next];
    tctx->next = (tctx->next + 1) % tctx->len;
    return ret;
}

// Cria um contexto de random de tabela
RandTable create_table_ctx(f64 *table, u32 len) {
    RandTable ret = {
        .ctx = {
            .uniform = table_uni,
            .type = RandType_Table,
        },
        .table = table,
        .len = len,
        .next = 0,
    };
    return ret;
}

// Gera um valor uniforme a partir de qualquer RandCtx
f64 randUniform(RandCtx *ctx) {
    return ctx->uniform(ctx);
}

// Gera um valor exponencial a partir de qualquer RandCtx
f64 randExp(RandCtx *ctx, f64 lambda) {
    assert( lambda > 0 );
    f64 val = randUniform(ctx);
    return -1.0f * (log(val) / lambda);
}

#ifdef RAND_MAIN
#undef RAND_MAIN

#include <time.h>

#define STATS_IMPL
#include "stats.c"

#include "test.c"

// main para testar os valores pseudoaleatorios
int main() {
    RandCtx rand_ctx = create_rand_ctx((u32) time(NULL)),
            *ctx = &rand_ctx;
    const u32 n = 100000;

    // Teste para gerar valores pseudoaleatorios da uniforme
    SECTION("RANDC Uniform");
    Stats stat = new_stats();

    for ( u32 i = 0; i < n; i++ ) {
        const f64 val = randUniform(ctx);
        acc_and_update(&stat, val, 1);
    }

    const f64 avgUni = discrete_average(stat),
          varUni = discrete_variance(stat);
    const f64 uniavg = 0.5f, univar = 1.0f / 12.0f;
    f64_expect_equal_tol("RANDC uniform average",
            uniavg, avgUni, 0.01);
    f64_expect_equal_tol("RANDC uniform variance",
            univar, varUni, 0.01);
    log("   avg: %7.7lf,    var: %7.7lf\n", avgUni, varUni);
    log("uniavg: %7.7lf, univar: %7.7lf\n\n", uniavg, univar);

    // Teste para gerar valores pseudoaleatorios da exponencial
    SECTION("RANDC Exponential");
    stat = new_stats();
    const f64 lambda = 7.3f;
    for ( u32 i = 0; i < n; i++ ) {
        const f64 val = randExp(ctx, lambda);
        acc_and_update(&stat, val, 1);
    }

    const f64 avgExp = discrete_average(stat),
          varExp = discrete_variance(stat);
    const f64 expavg = 1 / lambda, expvar = expavg * expavg;
    f64_expect_equal_tol( "RANDC exponential average",
            expavg, avgExp, 0.01);
    f64_expect_equal_tol("RANDC exponential variance",
            expvar, varExp, 0.01);
    log("   avg: %7.7lf,    var: %7.7lf\n", avgExp, varExp);
    log("expavg: %7.7lf, expvar: %7.7lf\n\n", expavg, expvar);

    end_tests("Random RANDC");

    // Teste da tabela para gerar valores pseudoaleatorios
    SECTION("Random Table");
    u32 table_len = 40;
    f64 table[table_len];
    for ( u32 i = 0; i < table_len; i++ ) {
        table[i] = ((f64) i) / table_len;
    }
    RandTable rand_table = create_table_ctx(table, table_len);
    RandCtx *tctx = (RandCtx *) &rand_table;
    log("table output:");
    for ( u32 i = 0; i < 2 * table_len; i++ ) {
        const f64 expected = ((f64) (i % table_len)) / table_len;
        const f64 val = randUniform(tctx);
        f64_expect_equal("Table", expected, val);
        if ( i % 4 == 0 ) {
            log("\n");
        }
        log(" %7.7lf", val);
    }
    log("\n");

    end_tests("Random Table");

}
#endif // RAND_MAIN
#endif // RAND_IMPL
