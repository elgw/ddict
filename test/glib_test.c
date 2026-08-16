// See https://docs.gtk.org/glib/struct.HashTable.html

// For the test suite, we could have let glib use `g_direct_hash` which
// would be even faster. But that would not make a fair comparison.


#include "common.h"

#include <glib.h>
#include <glib-object.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;


static u32
_ddict_wordhash(const void * _word)
{
    const char * word = (const char *) _word;
    // your standard Polynomial rolling hash function
    u8 * bytes = (u8*) word;
    u64 h = 0;
    while(*bytes != '\0') {
        h = h*31 + *bytes++;
    }
    //printf("%s -> %lu\n", word, h);
    return h;
}


int main(int argc, char ** argv)
{
    u64 N = 100000;
    if(argc > 1) {
        N = atol(argv[1]);
    }

    printf("Inserting '0', '1', ..., 'N-1 (N=%lu)'\n", N);
    struct timespec t0, t1, t2, t3;

    // Figure out how large buffer we will need;
    // N = 1 -> "0\n", i.e. 2
    // N = 2 -> "0\n1\n", i.e. 4 etc
    size_t kss = N; // one '\0' per number
    {
        size_t t = 1;
        while(t <=10*N)
        {
            if(t <= N)
                kss += (N+1-t);
            t*=10;
        }
    }

    clock_gettime(CLOCK_REALTIME, &t0);

    // g_direct_hash would be faster and would work here,
    // however that would be comparing different things.

    GHashTable* H = g_hash_table_new (_ddict_wordhash,
                                      g_str_equal);

    char * key_storage = malloc(kss);
    char * key_write = key_storage;

    for(u64 n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        g_hash_table_insert(H, key_write, NULL);
        key_write += nwritten + 1;
    }

    clock_gettime(CLOCK_REALTIME, &t1);
    printf("Insert: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t1, &t0),
           1000000000.0 * timespec_diff(&t1, &t0) / (double) N);

    printf("Assuring that '0', '1', ..., 'n-1' are in the dict\n");
    clock_gettime(CLOCK_REALTIME, &t2);

    key_write = key_storage;
    for(u64 n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        if(g_hash_table_contains(H, key_write) == 0)
        {
            printf("Failed to retrieve '%s'\n", key_write);
            exit(EXIT_FAILURE);
        };
        key_write += nwritten + 1;
    }
    g_hash_table_destroy(H);
    free(key_storage);
    clock_gettime(CLOCK_REALTIME, &t3);

    printf("Scan: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t3, &t2),
           1000000000.0 * timespec_diff(&t3, &t2) / (double) N);
    printf("Total: %.3f ms\n",
           1000.0 * (timespec_diff(&t1, &t0) + timespec_diff(&t3, &t2)));;

    print_peak_mem();
    return EXIT_SUCCESS;
}
