#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <cdict/cdict.h>

static double
timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}

static int
get_peak_memory_KB(size_t * _VmPeak, size_t * _VmHWM)
{
    *_VmPeak = 0;
    *_VmHWM = 0;

    const char fname[] = "/proc/self/status";
    FILE * sf = fopen(fname, "r");
    if(sf == NULL) {
        fprintf(stderr, "Failed to open %s\n", fname);
        return 1;
    }

    const size_t buffsize = 1024;
    char * VmPeak = malloc(buffsize);
    if(VmPeak == NULL) {
        fclose(sf);
        return 1;
    }
    char * VmHWM = malloc(buffsize);
    if(VmHWM == NULL) {
        fclose(sf);
        free(VmPeak);
        return 1;
    }

    char * line = NULL;
    size_t len = 0;

    while( getline(&line, &len, sf) > 0) {
        if(strlen(line) > 6) {
            if(strncmp(line, "VmPeak", 6) == 0) {
                snprintf(VmPeak, buffsize, "%s", line);
            } else {
                if(strncmp(line, "VmHWM", 5) == 0) {
                    snprintf(VmHWM, buffsize, "%s", line);
                }
            }
        }
    }
    free(line);
    fclose(sf);

    // Assumes that the lines end with the 4 bytes ' kb \n'
    if(strlen(VmPeak) > 5) {
        VmPeak[strlen(VmPeak) - 4] = '\0';
        *_VmPeak = (size_t) atol(VmPeak+7);
    }

    if(strlen(VmHWM) > 5) {
        VmHWM[strlen(VmHWM) - 4] = '\0';
        *_VmHWM = (size_t) atol(VmHWM+7);
    }

    free(VmHWM);
    free(VmPeak);
    return 0;
}


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

    for(uint64_t n = 0; n < N; n++)
    {
        sprintf(word, "%lu", n);
        CDict_set_with_types(c, word, CDICT_DATA_TYPE_STRING,
                             0, CDICT_DATA_TYPE_INT);
    }
    clock_gettime(CLOCK_REALTIME, &t1);
    printf("Insert: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t1, &t0),
           1000000000.0 * timespec_diff(&t1, &t0) / (double) N);

    printf("Assuring that '0', '1', ..., 'n-1' are in the dict\n");
    clock_gettime(CLOCK_REALTIME, &t2);

    for(uint64_t n = 0; n < N; n++)
    {
        sprintf(word, "%lu", n);
        assert(CDict_nest_get_entry(c, word) != NULL);
    }

    clock_gettime(CLOCK_REALTIME, &t3);

    printf("Scan: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t3, &t2),
           1000000000.0 * timespec_diff(&t3, &t2) / (double) N);
    printf("Total: %.3f ms\n",
           1000.0 * (timespec_diff(&t1, &t0) + timespec_diff(&t3, &t2)));;

    CDict_free_and_free_contents(c);
    size_t VmPeak, VmHWM;
    if(get_peak_memory_KB(&VmPeak, &VmHWM) == 0){
        printf("\n");
        printf("VmPeak: %zu kb, VmHWM: %zu kb\n", VmPeak, VmHWM);
    }
    return EXIT_SUCCESS;

}
