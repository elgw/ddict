#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "ddict.h"

typedef uint8_t u8;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Enable to see some timings
// #define  DDICT_TIMINGS

#define DDICT_INITIAL_SIZE 8

#ifdef DDICT_TIMINGS
static double timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}
#endif


static char *
ddict_strdup(const char *str)
{
    size_t n = strlen(str) + 1;
    char * cpy = malloc(n);
    if(cpy == NULL) {
        return NULL;
    }
    return memcpy (cpy, str, n);
}

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

static inline i64
_ddict_entry_id_from_index(const ddict * dict, u64 idx)
{
    switch(dict->index_type)
    {
    case I8:
        return (i64) dict->indices8[idx];
    case I16:
        return (i64) dict->indices16[idx];
    case I32:
        return (i64) dict->indices32[idx];
    case I64:
        return (i64) dict->indices64[idx];
    }
    assert(0);
    return -1;
}

static inline i64
_ddict_get_free_index(const ddict * dict, u64 idx) {

    switch(dict->index_type)
    {
    case I8:
        while(dict->indices8[idx] != -1) {
            idx++;
            if(idx == dict->n_indices) {
                idx = 0;
            }
#ifdef DDICT_STATS
            dict->n_collisions++;
#endif
        }
        break;
    case I16:
        while(dict->indices16[idx] != -1) {
            idx++;
            if(idx == dict->n_indices) {
                idx = 0;
            }
#ifdef DDICT_STATS
            dict->n_collisions++;
#endif
        }
        break;
    case I32:
        while(dict->indices32[idx] != -1) {
            idx++;
            if(idx == dict->n_indices) {
                idx = 0;
            }
#ifdef DDICT_STATS
            dict->n_collisions++;
#endif
        }
        break;
    case I64:
        while(dict->indices64[idx] != -1) {
            idx++;
            if(idx == dict->n_indices) {
                idx = 0;
            }
#ifdef DDICT_STATS
            dict->n_collisions++;
#endif
            break;
        }
    }
    return idx;
}

static inline void
_ddict_set_index(ddict * dict, i64 idx, i64 value)
{
    switch(dict->index_type) {
    case I8:
        dict->indices8[idx] = value;
        return;
    case I16:
        dict->indices16[idx] = value;
        return;
    case I32:
        dict->indices32[idx] = value;
        return;
    case I64:
        dict->indices64[idx] = value;
        return;
    }
}

// Create the indices array, selecting the appropriate
// element size depending on the number of indices
static void
_ddict_gen_indices(ddict * dict) {
    // select data type
    dict->index_type = I8;
    if(dict->n_indices > 256-2){ dict->index_type = I16; }
    if(dict->n_indices > 65025-2){ dict->index_type = I32; }
    if(dict->n_indices > 4294967296-2){ dict->index_type = I64; }

    // allocate array
    switch(dict->index_type)
    {
    case I8:
        dict->indices8 = malloc(dict->n_indices*sizeof(i8));
        break;
    case I16:
        dict->indices16 = malloc(dict->n_indices*sizeof(i16));
        break;
    case I32:
        dict->indices32 = malloc(dict->n_indices*sizeof(i32));
        break;
    case I64:
        dict->indices64 = malloc(dict->n_indices*sizeof(i64));
        break;
    }

    if(dict->indices8 == NULL) {
        assert(0);
        exit(EXIT_FAILURE);
    }

    // set to -1
    switch(dict->index_type)
    {
    case I8:
        for(u64 kk = 0; kk < dict->n_indices; kk++) {
            dict->indices8[kk] = -1;
        }
        break;
    case I16:
        for(u64 kk = 0; kk < dict->n_indices; kk++) {
            dict->indices16[kk] = -1;
        }
        break;
    case I32:
        for(u64 kk = 0; kk < dict->n_indices; kk++) {
            dict->indices32[kk] = -1;
        }
        break;
    case I64:
        for(u64 kk = 0; kk < dict->n_indices; kk++) {
            dict->indices64[kk] = -1;
        }
        break;
    }
    return;
}

ddict * ddict_new_with_size(int manage_keys,
                            u64 n_indices,
                            int alloc_entries)
{
    if(n_indices < 8) {
        return NULL;
    }

    ddict * dict = calloc(1, sizeof(ddict));
    if(dict == NULL) {
        return NULL;
    }
    dict->manage_keys = manage_keys;
    dict->n_indices = n_indices;
    _ddict_gen_indices(dict);
    if(alloc_entries)
    {
        dict->n_entries_alloc = n_indices / 2;
        dict->entries = malloc(dict->n_entries_alloc*sizeof(ddict_entry));
        if(dict->entries == NULL)
        {
            free(dict->indices8); // no need to swith
            free(dict);
            return NULL;
        }
    }
    return dict;
}

ddict *
ddict_new(int manage_keys) {
    return ddict_new_with_size(manage_keys, DDICT_INITIAL_SIZE, 1);
}

void
ddict_free(ddict * dict) {
    if(dict == NULL) {
        return;
    }
    if(dict->manage_keys) {
        for(u64 kk = 0; kk < dict->n_entries; kk++) {
            free(dict->entries[kk].key);
        }
    }
    free(dict->entries);
    free(dict->indices8);
    free(dict);
    return;
}


static ddict_entry *
ddict_get_with_hash(const ddict * dict,
                    const char * key, const u64 hash)
{
    u64 idx = hash % dict->n_indices;
    // Until found or an empty slot appears
    while(1)
    {
        if(idx == dict->n_indices) {
            idx = 0; // wrap around
        }

        // i64 eid = dict->indices[idx]; // ddict_entry index
        i64 eid = _ddict_entry_id_from_index(dict, idx);
        if(eid == -1) {
            return NULL;
        }

#ifndef DDICT_DROP_HASH
        if(dict->entries[eid].hash != hash)
        {
            idx++;
            continue;
        }
#endif

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

ddict_entry *
ddict_get(const ddict * dict,
          const char * key)
{
    const u64 hash = wordhash(key);
    return ddict_get_with_hash(dict, key, hash);
}


// Make room for more entries, This can have a separate growth rate
// compared to the index as it does not matter if it is almost full.
static void
ddict_grow_entries(ddict * dict)
{
    //printf("!!! Growing entries\n");
    dict->n_entries_alloc = 1.5 * dict->n_entries_alloc;
    dict->entries = realloc(dict->entries,
                            dict->n_entries_alloc*sizeof(ddict_entry));
    if(dict->entries == NULL)
    {
        exit(EXIT_FAILURE);
    }
    return;
}


static void
ddict_grow_indices(ddict * dict)
{
    //printf("!!! Growing indices (%lu)\n", dict->n_indices);
    // Procedure
    // - create a new dict (dict2) that shares the entries from the original dict
    // - in dict2, index all entries present in the original dict
    // - transfer the index in dict2 to the original dict

#ifdef DDICT_TIMINGS
    struct timespec t0, t1;
    clock_gettime(CLOCK_REALTIME, &t0);
#endif
    assert(dict->n_indices > 0);
    u64 n_indices = dict->n_indices;
    u64 n_indices2 = n_indices*2;
    //    printf("Grow %lu -> %lu\n", n_indices, n_indices2);
    free(dict->indices8);
    ddict * dict2 = ddict_new_with_size(dict->manage_keys, n_indices2, 0);
    assert(dict2 != NULL);
    dict2->entries = dict->entries;
    dict2->n_entries_alloc = dict->n_entries_alloc;

    for(u64 kk = 0; kk < dict->n_entries; kk++)
    {
        ddict_entry e = dict->entries[kk];
#ifdef DDICT_DROP_HASH
        u64 idx = wordhash(e.key) % n_indices2;
#else
        u64 idx = e.hash % n_indices2;
#endif
        // Find a free index slot
        idx = _ddict_get_free_index(dict2, idx);
        // Make it point to the already present entry
        _ddict_set_index(dict2, idx, kk);
    }

    // Note: No need to switch since all point to the same memory location
    dict->indices64 = dict2->indices64;
    dict->index_type = dict2->index_type;

#ifdef DDICT_STATS
    dict->n_collisions = dict2->n_collisions;
#endif
    dict->n_indices = dict2->n_indices;
    // We steal the indices from dict2 and it never had an entry
    // so we can just free the struct
    free(dict2);
#ifdef DDICT_TIMINGS
    clock_gettime(CLOCK_REALTIME, &t1);
    printf("grow indices took %f s\n", timespec_diff(&t1, &t0));
#endif

    return;
}


// probably we want a ddict_set as well, which updates existing
// elements
int
ddict_add(ddict * dict,
          char * word, void * value)
{
    const u64 hash = wordhash(word);

    // See if in dictionary
    if(ddict_get_with_hash(dict, word, hash) != NULL) {
        return 1;
    }

    // Increase capacities if needed
    if(dict->n_entries == dict->n_entries_alloc) {
        ddict_grow_entries(dict);
    }
    if(2*dict->n_entries > dict->n_indices) {
        ddict_grow_indices(dict);
    }

    // Add at the end of the list of entries
#ifndef DDICT_DROP_HASH
    dict->entries[dict->n_entries].hash = hash;
#endif
    if(dict->manage_keys) {
        dict->entries[dict->n_entries].key = ddict_strdup(word);
    } else {
        dict->entries[dict->n_entries].key = word;
    }

    dict->entries[dict->n_entries].value = value;

    // Figure out where we can insert a reference
    u64 idx = hash % dict->n_indices;
    idx = _ddict_get_free_index(dict, idx);
    _ddict_set_index(dict, idx, dict->n_entries);
    //printf("idx = %lu\n", idx);
    dict->n_entries++;
    return 0;
}

int
ddict_size(const ddict * dict)
{
    return dict->n_entries;
}

int ddict_update_entry(ddict * dict, const char * key, void * value)
{
    ddict_entry * e;
    if( (e = ddict_get(dict, key)) == NULL)
    {
        return 1;
    }
    e->value = value;
    return 0;
}
