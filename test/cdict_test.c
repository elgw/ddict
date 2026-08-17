#include "common.h"
#include "cdict.h"

typedef char* string;
CDict(string, int) cdict_t;

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

    // Instance of cdict_int_t;
    cdict_t cdict_instance;

    // Initialize
    cdict__init(&cdict_instance);

    char * key_storage = malloc(kss);
    char * key_write = key_storage;

    // Add elements
    for(u64 n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        cdict__add(&cdict_instance, key_write, 0);
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
        int value;
        if(cdict__get(&cdict_instance, key_write, &value) == 0) {
            printf("Failed to retrieve '%s'\n", key_write);
            exit(EXIT_FAILURE);
        };
        key_write += nwritten + 1;
    }
    cdict__clear(&cdict_instance);
    cdict__free(&cdict_instance);
    free(key_storage);
    clock_gettime(CLOCK_REALTIME, &t3);

    printf("Scan: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t3, &t2),
           1000000000.0 * timespec_diff(&t3, &t2) / (double) N);
    printf("Total: %.3f ms\n",
           1000.0 * (timespec_diff(&t1, &t0) + timespec_diff(&t3, &t2)));;

    print_peak_mem();
    return EXIT_SUCCESS;




}
