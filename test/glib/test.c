// See https://docs.gtk.org/glib/struct.HashTable.html

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <glib.h>
#include <glib-object.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

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

    clock_gettime(CLOCK_REALTIME, &t0);

    // g_direct_hash would be faster and would work here,
    // however that would be comparing different things.

    GHashTable* H = g_hash_table_new (_ddict_wordhash,
                                      g_str_equal);

    size_t key_storage_size = 9*N;
    char * key_storage = malloc(key_storage_size);
    char * key_write = key_storage;

    for(u64 n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        g_hash_table_insert(H, key_write, NULL);
        key_write += nwritten + 1;
        assert(key_write  < key_storage + key_storage_size);
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
    clock_gettime(CLOCK_REALTIME, &t3);

    printf("Scan: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t3, &t2),
           1000000000.0 * timespec_diff(&t3, &t2) / (double) N);
    printf("Total: %.3f ms\n",
           1000.0 * (timespec_diff(&t1, &t0) + timespec_diff(&t3, &t2)));;
    g_hash_table_destroy(H);
    free(key_storage);
    size_t VmPeak, VmHWM;
    if(get_peak_memory_KB(&VmPeak, &VmHWM) == 0){
        printf("\n");
        printf("VmPeak: %zu kb, VmHWM: %zu kb\n", VmPeak, VmHWM);
    }
    return EXIT_SUCCESS;
}
