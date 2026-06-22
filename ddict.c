#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "ddict.h"

typedef uint8_t u8;
typedef int32_t i32;
typedef uint64_t u64;

// Enable to see some timings
// #define  DDICT_TIMINGS

#ifdef DDICT_TIMINGS
static double timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}
#endif

static u64
wordhash(const char * word)
{
    u8 * bytes = (u8*) word;
    u64 h = 0;
    while(*bytes++ != '\0') {
        h = h*31 + *bytes;
    }
    return h;
}

ddict *
ddict_new() {
    ddict * dict = calloc(1, sizeof(ddict));
    if(dict == NULL) {
        return NULL;
    }
    dict->n_indices = 8;
    dict->indices = malloc(dict->n_indices*sizeof(i32));
    if(dict->indices == NULL) {
        free(dict);
        return NULL;
    }
    for(u64 kk = 0; kk < dict->n_indices; kk++)
    {
        dict->indices[kk] = -1;
    }
    dict->n_entries_alloc = 4;
    dict->entries = malloc(dict->n_entries_alloc*sizeof(entry));
    if(dict->entries == NULL)
    {
        free(dict->indices);
        free(dict);
        return NULL;
    }
    return dict;
}


void
ddict_free(ddict * dict) {
    assert(dict != NULL);
    for(u64 kk = 0; kk < dict->n_entries; kk++) {
        free(dict->entries[kk].key);
    }
    free(dict->entries);
    free(dict->indices);
    free(dict);
}


static entry *
ddict_get_with_hash(const ddict * dict,
                    const char * key, const u64 hash)
{
    u64 idx = hash % dict->n_indices;
    while(1)
    {
        if(idx == dict->n_indices) {
            idx = 0;
        }

        assert(idx < dict->n_indices);

        i32 eid = dict->indices[idx]; // entry index
        if(eid == -1) {
            return NULL;
        }
        if(dict->entries[eid].hash != hash)
        {
            idx++;
            continue;
        }
        if(strcmp(key, dict->entries[eid].key) == 0)
        {
            return &(dict->entries[eid]);
        }
        idx++;
    }
    assert(0);
    __builtin_unreachable();
    return NULL;
}

entry *
ddict_get(const ddict * dict,
          const char * key)
{
    const u64 hash = wordhash(key);
    return ddict_get_with_hash(dict, key, hash);
}


static void
ddict_grow_entries(ddict * dict)
{
    //printf("Entries allocation: %d -> %d\n", dict->n_entries_alloc,
    //(int) (1.5 * dict->n_entries_alloc));
    dict->n_entries_alloc = 1.5 * dict->n_entries_alloc;
    dict->entries = realloc(dict->entries,
                            dict->n_entries_alloc*sizeof(entry));
    assert(dict->entries != NULL);
    return;
}


static void
ddict_grow_indices(ddict * dict)
{

#ifdef DDICT_TIMINGS
    struct timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);
#endif
    assert(dict->n_indices > 0);
    u64 n_indices = dict->n_indices;
    u64 n_indices2 = n_indices*2;
    //printf("indices %d -> %d\n", n_indices, n_indices2);
    i32 * indices2 = malloc(n_indices2*sizeof(i32));
    for(u64 kk = 0; kk < n_indices2; kk++) {
        indices2[kk] = -1;
    }
    dict->n_collisions = 0;

    for(u64 kk = 0; kk < dict->n_entries; kk++)
    {
        entry e = dict->entries[kk];
        u64 idx = e.hash % n_indices2;
        // Linear probing
        while(indices2[idx] != -1) {
            idx++;
            if(idx == n_indices2) {
                idx = 0;
            }
            dict->n_collisions++;
        }
        indices2[idx] = kk;
    }

    free(dict->indices);
    dict->indices = indices2;
    dict->n_indices = n_indices2;
#ifdef DDICT_TIMINGS
    clock_gettime(CLOCK_REALTIME, &t1);
    printf("grow indices took %f s\n", timespec_diff(&t1, &t0));
#endif
}


// probably we want a ddict_set as well, which updates existing
// elements
int
ddict_add(ddict * dict,
          const char * word, void * value)
{
    const u64 hash = wordhash(word);

    // See if in dictionary
    if(ddict_get_with_hash(dict, word, hash) != NULL)
    {
        return 1;
    }
    if(dict->n_entries == dict->n_entries_alloc) {
        ddict_grow_entries(dict);
    }
    if(2*dict->n_entries > dict->n_indices) {
        ddict_grow_indices(dict);
    }

    dict->entries[dict->n_entries].hash = hash;
    dict->entries[dict->n_entries].key = strdup(word);
    dict->entries[dict->n_entries].value = value;

    u64 idx = hash % dict->n_indices;
    assert(idx < dict->n_indices);
    while(dict->indices[idx] != -1) {
        idx++;
        if(idx == dict->n_indices) {
            idx = 0;
        }
        dict->n_collisions++;
    }
    dict->indices[idx] = dict->n_entries;
    dict->n_entries++;
    return 0;
}
