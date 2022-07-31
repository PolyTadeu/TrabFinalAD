#include <stdio.h>
#include <assert.h>

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
    u8 verbose;
    u8 ARGS_valid;
} Options;

int gen_struct() {
    ARGS_struct_from_spec(spec);
    return 0;
}

int main(const int argc, const char **argv) {
    Options opts = {
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
    printf("    .verbose = %hhu\n", opts.verbose);
    printf("    .ARGS_valid = %hhu\n", opts.ARGS_valid);
    printf("}\n");

    return 0;
}
