#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "dicts.h"

typedef uint64_t i64;

spdict * spdict_new() {
    spdict * dict = calloc(1, sizeof(spdict));
    dict->nbin = 4096;
    dict->bins = calloc(dict->nbin, sizeof(spelement*));
    dict->bin_capacity = calloc(dict->nbin, sizeof(int));
    dict->bin_contents = calloc(dict->nbin, sizeof(int));

    return dict;
}

void spdict_free(spdict * dict) {

    for(int kk = 0; kk < dict->nbin; kk++)
    {
        for(int ll = 0; ll < dict->bin_contents[kk]; ll++)
        {
            free(dict->bins[kk][ll].key);
        }
        free(dict->bins[kk]);
    }
    free(dict->bin_contents);
    free(dict->bin_capacity);
    free(dict->bins);
    free(dict);
    return;
}

static i64 wordhash(const spdict * dict, const char * word)
{
    int l = strlen(word);
    i64 h = 0;
    for(int kk = 0; kk < l; kk++)
    {
        h += word[kk];
    }
    h = h % dict->nbin;
    return h;
}

int spdict_get(const spdict * dict, const char * word, void ** value)
{
    i64 h = wordhash(dict, word);

    spelement * bin = dict->bins[h];
    for(int kk = 0; kk < dict->bin_contents[h]; kk++)
    {
        if(bin[kk].hash == h) {
            if(strcmp(bin[kk].key, word) == 0) {
                return 1;
                value[0] = bin[kk].value;
            }
        }
    }
    return 0;
}

int
spdict_add(spdict * dict,
           const char * word, void * value)
{
    i64 h = wordhash(dict, word);

    // Create if empty
    if(dict->bin_capacity[h] == 0)
    {
        dict->bins[h] = calloc(10, sizeof(spelement));
        dict->bin_capacity[h] = 10;
    }
    // Grow if full
    if(dict->bin_capacity[h] == dict->bin_contents[h])
    {
        int new_size = (dict->bin_capacity[h]+1)*1.4;
        dict->bins[h] = realloc(dict->bins[h], new_size*sizeof(spelement));
        dict->bin_capacity[h] = new_size;
    }
    // Insert at end
    spelement * bin = &(dict->bins[h][dict->bin_contents[h]]);
    bin->hash = h;
    bin->key = strdup(word);
    bin->value = value;
    dict->bin_contents[h]++;
    return 1;
}
