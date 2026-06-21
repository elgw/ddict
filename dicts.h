#pragma once

#include <stdlib.h>

typedef struct {
    char * key;
    void * value;
} spelement;

typedef struct {
    int nbin;
    spelement ** bins;
    int * bin_capacity;
    int * bin_contents;
} spdict;

spdict * spdict_new();

void spdict_free(spdict * dict);

int
spdict_get(const spdict * dict,
               const char * word, void ** value);

int
spdict_add(spdict * dict,
           const char * word, void * value);
