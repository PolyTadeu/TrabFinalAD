#ifndef HISTOGRAM_HEADER
#define HISTOGRAM_HEADER
#include "types.h"

typedef struct _HistNode {
    u32 key;
    f64 data;
    struct _HistNode *next;
    struct _HistNode *prev;
} HistNode;

typedef struct _Histogram {
    u32 max_key;
    u32 vec_len;
    f64 *data;
    HistNode *start;
    HistNode *cache;
    f64 total_sum;
} Histogram;

typedef struct _Pmf {
    u32 len;
    f64 *data;
} Pmf;

Histogram init_histogram(u32 vec_len);
void deinit_histogram(Histogram *h);
void insert_histogram(Histogram *h, u32 key, f64 value);
Pmf pmf_from_histogram(Histogram* h);

#endif // HISTOGRAM_HEADER

#ifdef HISTOGRAM_IMPL
#undef HISTOGRAM_IMPL
#include <assert.h>
#include <stdlib.h>

HistNode* new_histNode(u32 key, HistNode *prev) {
    HistNode *retp = malloc(sizeof(*retp));
    assert( retp );
    HistNode hist = {
        .key = key,
        .data = 0,
        .next = 0,
        .prev = prev,
    };
    *retp = hist;
    return retp;
}

void delete_histNodes(HistNode *node) {
    assert( !node->prev );
    while ( node->next ) {
        HistNode *next = node->next;
        free(node);
        node = next;
    }
}

Histogram init_histogram(u32 vec_len) {
    const Histogram ret = {
        .max_key = 0,
        .vec_len = vec_len,
        .data = malloc(
                sizeof(*(((Histogram *)0)->data))*vec_len),
        .start = 0,
        .cache = 0,
        .total_sum = 0.0,
    };
    assert( ret.data );
    for ( u32 i = 0; i < vec_len; i++ ) {
        ret.data[i] = 0;
    }
    return ret;
}

void deinit_histogram(Histogram *h) {
    delete_histNodes(h->start);
    free(h->data);
}

void insert_histogram(Histogram *h, u32 key, f64 val) {
    const i32 cache_dist =
        key - (h->cache ? h->cache->key : h->vec_len-1);
    const i32 start_dist = key - h->vec_len;
    if ( key < h->vec_len ) {
        h->data[key] += val;
    } else if ( abs(cache_dist) <= abs(start_dist) ) {
        assert( -1 <= cache_dist && cache_dist <= 1 );
        assert( start_dist >= 0 );
        HistNode *node = h->cache;
        if ( cache_dist < 0 ) {
            while ( node->key > key ) {
                node = node->prev;
                assert( node );
                assert( node->key >= key );
            }
        } else {
            while ( node->key < key ) {
                if ( !(node->next) ) {
                    node->next =
                        new_histNode(node->key + 1, node);
                }
                node = node->next;
                assert( node->key >= key );
            }
        }
        assert( node->key == key );
        node->data += val;
        h->cache = node;
    } else {
        assert( key == h->vec_len );
        if ( !(h->start) ) {
            h->start = new_histNode(key, 0);
            assert( h->start );
        }
        h->start->data += val;
        h->cache = h->start;
    }
    h->total_sum += val;
    h->max_key = (h->max_key > key) ? h->max_key : key;
}

Pmf pmf_from_histogram(Histogram* h) {
    Pmf pmf = {
        .len = h->max_key+1,
        .data = malloc(
                sizeof(*(((Pmf*)0)->data))*(h->max_key+1)),
    };
    assert( pmf.data );
    u32 key = 0;
    f64 running_sum = 0;
    for ( u32 i = 0; i < h->vec_len; i++ ) {
        assert( key == i );
        pmf.data[key] = h->data[i] / h->total_sum;
        key += 1;
        running_sum += h->data[i];
    }
    for ( HistNode *n = h->start; n; n = n->next ) {
        assert( key == n->key );
        pmf.data[key] = n->data / h->total_sum;
        key += 1;
        running_sum += n->data;
    }
    assert( h->total_sum - running_sum < 1e-16 );
    return pmf;
}

void free_pmf(Pmf pmf) {
    free(pmf.data);
}

#ifdef HISTOGRAM_MAIN
#undef HISTOGRAM_MAIN
#include <stdio.h>

void print_histogram(Histogram *h) {
    printf("{");
    for ( u32 i = 0; i < h->vec_len; i++ ) {
        printf(" %6.4lf", h->data[i]);
    }
    printf(" }\n(");
    for ( HistNode *n = h->start; n; n = n->next ) {
        printf(" %6.4lf", n->data);
    }
    printf(" )\n");
}

void print_pmf(Pmf pmf) {
    printf("[");
    for ( u32 i = 0; i < pmf.len; i++ ) {
        printf(" %6.4lf", pmf.data[i]);
    }
    printf(" ]\n");
}

int main() {
    Histogram hist = init_histogram(2), *h = &hist;

    insert_histogram(h, 0, 0.5);
    insert_histogram(h, 1, 1.0);
    insert_histogram(h, 2, 2.0);
    insert_histogram(h, 3, 3.0);
    insert_histogram(h, 2, 2.0);
    insert_histogram(h, 1, 1.0);
    insert_histogram(h, 0, 0.5);

    print_histogram(h);

    Pmf pmf = pmf_from_histogram(h);
    print_pmf(pmf);

    free_pmf(pmf);
    deinit_histogram(h);
}

#endif // HISTOGRAM_MAIN

#endif // HISTOGRAM_IMPL
