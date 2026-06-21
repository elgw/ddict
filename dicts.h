#pragma once

#include <stdlib.h>

// String=key, pointer=value
// The dictionary owns the keys
// One allocation per bin
// one allocation per insertion
// this is probably as slow as it can be
//
// Would be nice to have
// - A more proper hash function
// - a way to iterate over the (key, value) pairs
// - element count
// - removal of items
// - some simple introspection, what is the distribution of elements per bin
// - timings!

typedef struct {
    char * key;
    void * value;
} spelement;

typedef struct {
    // number of bins
    int nbin;
    // pointers to the bins
    spelement ** bins;
    // allocated storage capacity of each bin
    int * bin_capacity;
    // number of elements inserted into each bin
    int * bin_contents;
} spdict;

spdict * spdict_new();

void spdict_free(spdict * dict);

// See if an element is already in the dictionary
int
spdict_get(const spdict * dict,
               const char * word, void ** value);

// Insert without checking if the key is already in the dictionary
int
spdict_add(spdict * dict,
           const char * word, void * value);
