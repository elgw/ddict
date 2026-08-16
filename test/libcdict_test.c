#include "common.h"

#include "libcdict/src/cdict.h"

int main(int argc, char ** argv)
{
    uint64_t N = 100000;
    if(argc > 1) {
        N = atol(argv[1]);
    }

    char word[1024];

    struct timespec t0, t1, t2, t3;

    printf("Inserting '0', '1', ..., '%lu (N=%lu)'\n", N-1, N);
    clock_gettime(CLOCK_REALTIME, &t0);
    CDict_new(c);

    for(uint64_t n = 0; n < N; n++) {
        sprintf(word, "%lu", n);
        CDict_set_with_types(c, word, CDICT_DATA_TYPE_STRING,
                             0, CDICT_DATA_TYPE_INT);
    }
    clock_gettime(CLOCK_REALTIME, &t1);
    printf("Insert: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t1, &t0),
           1000000000.0 * timespec_diff(&t1, &t0) / (double) N);

    printf("Assuring that '0', '1', ..., 'n-1' are in the dict\n");
    clock_gettime(CLOCK_REALTIME, &t2);

    for(uint64_t n = 0; n < N; n++) {
        sprintf(word, "%lu", n);
        if(CDict_nest_get_entry(c, word) == NULL) {
            printf("Failed to retrieve '%s'\n", word);
            exit(EXIT_FAILURE);
        }
    }

    CDict_free_and_free_contents(c);
    clock_gettime(CLOCK_REALTIME, &t3);

    printf("Scan: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t3, &t2),
           1000000000.0 * timespec_diff(&t3, &t2) / (double) N);
    printf("Total: %.3f ms\n",
           1000.0 * (timespec_diff(&t1, &t0) + timespec_diff(&t3, &t2)));;

    print_peak_mem();
    return EXIT_SUCCESS;
}
