#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

#include "ddict.h"

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

static void
print_peak_mem(){
    size_t VmPeak, VmHWM;
    if(get_peak_memory_KB(&VmPeak, &VmHWM) == 0){
        printf("\n");
        printf("VmPeak: %zu kb, VmHWM: %zu kb\n", VmPeak, VmHWM);
    }
    return;
}

static void
example1(void)
{
    printf("#\n# Example 1 -- basic usage\n#\n");
    ddict * dict = ddict_new(1);
    printf("dict_add['username'] = 'john_doe'\n");
    ddict_add(dict, "username", "john_doe");
    printf("dict_add['password'] = '1234'\n");
    ddict_add(dict, "password", "1234");
    printf("dict_add['password'] = '4567'\n");
    if(ddict_add(dict, "password", "4567"))
    {
        printf("Can not ADD 'password', already set\n");
        printf("dict_update['password'] = '4567'\n");
        ddict_update_entry(dict, "password", "4567");
    }

    ddict_entry * ans;
    if((ans = ddict_get(dict, "username"))) {
        printf("dict_get['username'] -> '%s'\n", (char*) ans->value);
    }
    if((ans = ddict_get(dict, "password"))) {
        printf("dict_get['password'] -> '%s'\n", (char*) ans->value);
    }
    if((ddict_get(dict, "height")) == NULL) {
        printf("dict_get['height'] -> ERROR: 'height' is not in the dictionary\n");
    }
    ddict_free(dict);
    return;
}

static void
example2(void)
{
    printf("#\n# Example 2 -- looping over entries\n#\n");
    ddict * dict = ddict_new(1);
    ddict_add(dict, "username", "john_doe");
    ddict_add(dict, "password", "1234");
    // Please note that the entries are offset by 1
    for(uint64_t kk = 1; kk <= ddict_size(dict); kk++)
    {
        ddict_entry * ent = &dict->entries[kk];
        printf("entry %lu/%lu key: '%s', value: '%s'\n", kk+1, ddict_size(dict),
               (char*) ent->key,
               (char*) ent->value);
    }
    ddict_free(dict);
    return;
}

typedef int64_t i64;

typedef struct {
    FILE * fid;
    char * word;
    i64 buf_size;
} wreader;

wreader * word_reader_new(const char * fname)
{
    wreader * reader = calloc(1, sizeof(wreader));
    if(reader == NULL) {
        return NULL;
    }
    reader->fid = fopen(fname, "r");
    if(reader->fid == NULL)
    {
        free(reader);
        fprintf(stderr, "Unable to open %s\n", fname);
        exit(EXIT_FAILURE);
    }
    reader->buf_size = 1024;
    reader->word = malloc(reader->buf_size);
    if(reader->word == NULL) {
        exit(EXIT_FAILURE);
    }
    return reader;
}

void
word_reader_free(wreader * reader)
{
    fclose(reader->fid);
    free(reader->word);
    free(reader);
    return;
}

int is_whitespace(const char c)
{
    switch(c){
    case ' ':
        return 1;
    case '\n':
        return 1;
    case '\t':
        return 1;
    case '\f':
        return 1;
    case '\r':
        return 1;
    }
    return 0;
}

int
word_reader_read(wreader * reader, char ** result)
{
    int c;
    int wpos = 0; // where to write in the output buffer

    while(1){
        if(wpos + 1 == reader->buf_size)
        {
            fprintf(stderr, "Out of buffer\n");
            exit(EXIT_FAILURE);
        }

        c = fgetc(reader->fid);
        if(c == EOF){
            if(wpos > 0) {
                goto return_word;
            }
            return 0;
        }

        if(wpos == 0) {
            if(is_whitespace(c)){
                continue;
            }
            reader->word[wpos++] = c;
            continue;
        }

        if(is_whitespace(c)){
            goto return_word;
        } else {
            reader->word[wpos++] = c;
            continue;
        }
        goto return_word;
    }

    assert(0);
    return 0;

 return_word:
    ;
    reader->word[wpos] = '\0';
    result[0] = reader->word;
    return 1;
}

ddict * dict_from_file(const char * dictfile)
{
    ddict * dict = ddict_new(1);

    printf("Reading lines from %s\n", dictfile);
    int n_dict = 0;
    char * line = NULL;
    size_t line_len = 0;
    FILE * fid = fopen(dictfile, "r");
    if(fid == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", dictfile);
        exit(EXIT_FAILURE);
    }
    while(getline(&line, &line_len, fid) != -1) {
        while(is_whitespace(line[strlen(line)-1]))
        { line[strlen(line)-1] = '\0'; }
        if(ddict_add(dict, line, NULL) == 0) {
            n_dict++;
        }
        assert(ddict_get(dict, line) != NULL);
    }

    free(line);
    fclose(fid);
    printf("Added %d words to the dictionary (dict_from_file)\n", n_dict);
    return dict;
}

// buffer[0] as well as each other position in the buffer that appear
// after a '\0' and not beeing '\0' is added to the dict
ddict * dict_from_buffer(const char * buffer, size_t buffer_len)
{
    ddict * dict = ddict_new(0);
    ddict_add(dict, (char*) &buffer[0], NULL);
    int n_dict = 1;
    for(size_t kk = 1; kk < buffer_len; kk++) {
        if(buffer[kk-1] == '\0'){
            if(buffer[kk] != '\0') {
                if(ddict_add(dict, (char*) &buffer[kk], NULL) == 0) {
                    n_dict++;
                    assert(ddict_get(dict, (char*) &buffer[kk]) != NULL);
                }
            }
        }
    }
    printf("Added %d words to the dictionary (dict_from_buffer)\n", n_dict);
    return dict;
}

char * read_and_split(const char * file, size_t * _file_size)
{
    FILE * fid = fopen(file, "r");
    if(fid == NULL) {
        return NULL;
    }
    fseek(fid, 0, SEEK_END);
    size_t file_size = ftell(fid);
    fseek(fid, 0, SEEK_SET);
    char * buf = malloc(file_size+1);
    assert(buf != NULL);
    ssize_t nread = fread(buf, 1, file_size, fid);
    if(nread != (ssize_t) file_size){
        exit(EXIT_FAILURE);
    }
    fclose(fid);
    *_file_size = file_size;
    // Replace blacks by '\0'
    for(size_t kk = 0; kk < file_size; kk++) {
        if((buf[kk] == '\n') | (buf[kk] == '\r')) {
            buf[kk] = '\0';
        }
    }
    buf[file_size] = '\0';
    return buf;
}

int
test_dict_keys(const char * dictfile, const char * txtfile, int manage_keys, int print_unknown)
{
    ddict * dict = NULL;
    char * key_buffer = NULL;
    struct timespec t0, t1, t2;
    if(manage_keys)
    {
        clock_gettime(CLOCK_REALTIME, &t0);
        dict = dict_from_file(dictfile);
        clock_gettime(CLOCK_REALTIME, &t1);
    } else {
        clock_gettime(CLOCK_REALTIME, &t0);
        size_t key_buffer_size;
        key_buffer = read_and_split(dictfile, &key_buffer_size);
        if(key_buffer == NULL) {
            exit(EXIT_FAILURE);
        }
        dict = dict_from_buffer(key_buffer, key_buffer_size);
        clock_gettime(CLOCK_REALTIME, &t1);
    }

    // 2. Find all words not in the dict from
    // the txtfile
    wreader * reader = word_reader_new(txtfile);
    if(reader == NULL)
    {
        exit(EXIT_FAILURE);
    }
    printf("Looking for unknown words in %s\n", txtfile);
    int n_known = 0;
    int n_unknown = 0;
    char * word;
    if(print_unknown) {
        while(word_reader_read(reader, &word))
        {
            if(ddict_get(dict, word) == NULL){
                printf("Unknown: %s\n", word);
                n_unknown++;
            } else {
                n_known++;
            }
        }
    } else {
        while(word_reader_read(reader, &word))
        {
            if(ddict_get(dict, word) == NULL){
                n_unknown++;
            } else {
                n_known++;
            }
        }
    }
    word_reader_free(reader);
    clock_gettime(CLOCK_REALTIME, &t2);


    // 3. Count the word frequency and print the 10 most common

    // todo... ?

    // done
    ddict_free(dict);
    free(key_buffer);
    printf("Found %d known and %d unknown words\n", n_known, n_unknown);

    printf("Construct: %.3f ms\n", 1000.0 * timespec_diff(&t1, &t0));
    printf("Scan: %.3f ms\n", 1000.0 * timespec_diff(&t2, &t1));
    printf("Total: %.3f ms\n", 1000.0 * timespec_diff(&t2, &t0));
    return 0;
}

static int
test_synthetic(u64 n)
{
    printf("Inserting \"0\", \"2\", ..., \"N-1\" (N=%lu)\n", n);
    struct timespec t0, t1, t2, t3;

    clock_gettime(CLOCK_REALTIME, &t0);
    ddict * dict = ddict_new(1);
    char word[64];
    for(u64 kk = 0; kk < n; kk++)
    {
        sprintf(word, "%lu", kk);
        if(ddict_add(dict, word, 0)) {
            printf("Failed to add '%s'\n", word);
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_REALTIME, &t1);
    printf("Insert: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t1, &t0),
           1000000000.0 * timespec_diff(&t1, &t0) / (double) n);

    printf("Assuring that \"0\", \"1\", ... are in the dict\n");
    clock_gettime(CLOCK_REALTIME, &t2);
    for(u64 kk = 0; kk < n; kk++)
    {
        sprintf(word, "%lu", kk);
        if( ddict_get(dict, word) == NULL) {
            printf("Failed to retrieve '%s'\n", word);
            exit(EXIT_FAILURE);
        }
    }
    ddict_free(dict);
    clock_gettime(CLOCK_REALTIME, &t3);

    printf("Scan: %.3f ms (avg: %.3f ns)\n", 1000.0 * timespec_diff(&t3, &t2),
           1000000000.0 * timespec_diff(&t3, &t2) / (double) n);
    printf("Total: %.3f ms\n",
           1000.0 * (timespec_diff(&t1, &t0) + timespec_diff(&t3, &t2)));;

    print_peak_mem();
    return EXIT_SUCCESS;
}

static int
test_dictionary(int argc, char ** argv)
{
    if(argc < 2) {
        printf("Error, two file names needed\n");
        return EXIT_FAILURE;
    }

    const char * dictfile = argv[0];
    const char * txtfile = argv[1];


    printf("\nTest 1/2\n");
#ifdef DDICT_DROP_HASH
    printf("-> Managed keys & DDICT_DROP_HASH\n");
#else
    printf("-> Managed keys, hash values cached\n");
#endif
    test_dict_keys(dictfile, txtfile, 1, 0);


    printf("\nTest 2/2\n");
#ifdef DDICT_DROP_HASH
    printf("-> External keys & DDICT_DROP_HASH\n");
#else
    printf("-> External keys, hash values cached\n");
#endif
    test_dict_keys(dictfile, txtfile, 0, 0);

    size_t VmPeak, VmHWM;
    if(get_peak_memory_KB(&VmPeak, &VmHWM) == 0){
        printf("\n");
        printf("VmPeak: %zu kb, VmHWM: %zu kb\n", VmPeak, VmHWM);
    }

    print_peak_mem();
    return EXIT_SUCCESS;
}

int
main(int argc, char ** argv)
{
    if(argc > 1){
        if(strcmp(argv[1], "--help") == 0)
        {
            printf("Usage:\n");
            printf("%s"
                   "\n\t Run the basic examples\n",
                   argv[0]);
            printf("%s N"
                   "\n\t Run the toy benchmark with N words\n",
                   argv[0]);
            printf("%s --dict dict.txt text.txt"
                   "\n\t Load the words from dicts.txt (one per line) and use it"
                   "\n\t to fund unknown words in text.txt\n",
                   argv[1]);
            exit(EXIT_SUCCESS);
        }
        if(strcmp(argv[1], "--dict") == 0)
        {
            return test_dictionary(argc-2, argv+2);
        }
    }

    if(argc == 1)
    {
        printf("For more uses, try running with --help\n");
        printf("\n");
        example1();
        example2();
        exit(EXIT_SUCCESS);
    }

    u64 n = atol(argv[1]);
    return test_synthetic(n);
}
