#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "types.h"
#include "test.c"

typedef struct {
    size_t len, size;
    i32 *data;
} Array;

void add(Array *arr, const i32 val) {
    assert( arr->size < arr->len );
    assert( arr->data );
    arr->data[arr->size] = val;
    arr->size += 1;
}

void check_cicles(const Array arr) {
    const size_t size = arr.size;
    size_t cicle = 0;
    for ( size_t i = 1; i < size ; i++ ) {
        if ( arr.data[0] == arr.data[i] ) {
            size_t j = 1;
            for ( ; i + j < size
                    && arr.data[i+j] == arr.data[j]; j++ ) {
            }
            cicle = cicle > j ? cicle : j;
        }
    }
    if ( cicle > 1 ) {
        log("Maximum cicle found with %zu values\n", cicle);
    } else if ( cicle == 1) {
        log("Found one value in commom with itself\n");
    } else {
        log("No cicle found with %zu values\n", size);
    }
}

Array test_seed(u32 seed, const Array *old, const size_t limit) {
    log("Generating 0x%zX values, with seed %u\n", limit, seed);
    srand(seed);
    Array new = {
        .len = limit,
        .size = 0,
        .data = malloc(sizeof(*(((Array *)0)->data))*limit),
    };
    assert( new.data );

    for ( size_t i = 0; i < limit; i++ ) {
        add(&new, rand());
    }

    check_cicles(new);

    if ( old ) {
        log("Comparing sequences:");
        b32 found_eq = 0;
        for ( size_t k = 0; k < old->size; k++ ) {
            for ( size_t i = 0; i < new.size; i++ ) {
                if ( new.data[i] == old->data[k] ) {
                    found_eq = 1;
                    log("\nFound equal at indexes:"
                            " new %zu, old %zu, value %d\n",
                            i, k, new.data[i]);
                    size_t j = 1;
                    for ( ; i + j < new.size
                            && k + j < old->size
                            && new.data[i+j] == old->data[k+j]
                            ; j++ ) {
                        // Empty
                    }
                    log("Extends for %zu values\n", j);
                    if ( j > 1 ) {
                        log("\n=== The sequence found ===\n");
                        for ( size_t l = 0; l < j; l++ ) {
                            log("%d%c", new.data[i+l],
                                    ((l+1) % 6 ? ' ' : '\n'));
                        }
                        log("\n");
                        err();
                    }
                }
            }
        }
        if ( !found_eq ) {
            log(" no matches found\n");
        }
    }
    return new;
}

int main() {
    SECTION("Seed overlap");
    const size_t limit = 0x10000;
    const Array one = test_seed(time(NULL), NULL, limit);
    const Array two = test_seed(time(NULL) + clock(), &one, limit);
    (void) two;
    end_tests("Seed overlap");
}
