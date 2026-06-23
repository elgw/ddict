#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

// A dense key storage dictionary data structure, similar to what is
// found in Python but less sophisticated
//
// - Grows dynamically when needed
//
// - Using Open Addressing with Linear Probing
//
// - Can either own all the keys, i.e. makes copies of them and make
// sure to free them at the end, or have the caller own the keys.
//
// - Only string keys, will be scanned until a '\0' is found
//
// - If the hash value is cheap to compute, there is no need to save
// them, disable hash storage by defining DDICT_DROP_HASH below.
//


// #define DDICT_DROP_HASH
// #define DDICT_STATS

typedef struct {
#ifndef DDDICT_DROP_HASH
    uint64_t hash;
#endif
    char * key;
    void * value;
} entry;



typedef struct {
    // Array of indices that points into the entries
    int32_t * indices;
    uint64_t n_indices;
    // Storage for (hash, key, value) triplets
    entry * entries;
    // Number of entries that are used
    uint64_t n_entries;
    // Total number of entries
    uint64_t n_entries_alloc;
#ifdef DDICT_STATS
    uint64_t n_collisions;
#endif
    // If the dict should allocate and store private copies
    // of the keys
    int manage_keys;
} ddict;

// Create a new dictionary with the default size
ddict * ddict_new(int manage_keys);

// Free the dictionary and all the key copies that it holds
void ddict_free(ddict * dict);

// Return an entry with the given key or NULL if
// nothing is found
entry * ddict_get(const ddict *, const char * key);

// Add an entry to the dictionary unless it already contains
// the given key.
//
// Returns 0 on success.
int
ddict_add(ddict * dict,
          char * key, void * value);

#ifdef __cplusplus
}
#endif
