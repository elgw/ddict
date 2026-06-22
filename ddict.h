#pragma once

#include <stdlib.h>
#include <stdint.h>

// Dense dictionary, similar to (but less sophisticated) what is found in Python
// Owns all the keys (makes copies of them and frees them at exit)
// Grows dynamically when needed
// Using Open Addressing with Linear Probing
//
// keys will be scanned until a '\0' is found
//
// They key copying can probably be significanlty faster
// if we use an arena/pool allocator/allocation.


typedef struct {
    uint64_t hash;
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
    // Not used ...
    uint64_t n_collisions;
} ddict;

// Create a new dictionary with the default size
ddict * ddict_new();

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
          const char * key, void * value);
