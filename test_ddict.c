#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <string.h>

#include "ddict.h"

static double timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}

static int
get_peak_memory_KB(size_t * _VmPeak, size_t * _VmHWM)
{
    FILE * sf = fopen("/proc/self/status", "r");
    if(sf == NULL)
    {
        fprintf(stderr, "Failed to open /proc/self/status\n");
        return 1;
    }
    *_VmPeak = 0;
    *_VmHWM = 0;

    char * VmPeak = NULL;
    char * VmHWM = NULL;

    char * line = NULL;
    size_t len = 0;

    while( getline(&line, &len, sf) > 0)
    {
        if(strlen(line) > 6)
        {
            if(strncmp(line, "VmPeak", 6) == 0)
            {
                free(VmPeak);
                VmPeak = strdup(line);
            }
            if(strncmp(line, "VmHWM", 5) == 0)
            {
                free(VmHWM);
                VmHWM = strdup(line);
            }
        }
    }
    free(line);
    fclose(sf);

    if((VmPeak != NULL) && (strlen(VmPeak) > 11))
    {
        VmPeak[strlen(VmPeak) - 4] = '\0';

        //    printf("peakline: '%s'\n", peakline+7);
        *_VmPeak = (size_t) atol(VmPeak+7);
    }
    free(VmPeak);

    if((VmHWM != NULL) && (strlen(VmHWM) > 10))
    {
        VmHWM[strlen(VmHWM) - 4] = '\0';

        //    printf("peakline: '%s'\n", peakline+7);
        *_VmHWM = (size_t) atol(VmHWM+7);
    }
    free(VmHWM);

    return 0;
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
    reader->fid = fopen(fname, "r");
    if(reader->fid == NULL)
    {
        free(reader);
        return NULL;
    }
    reader->buf_size = 1024;
    reader->word = malloc(reader->buf_size);
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
    while(getline(&line, &line_len, fid) != -1) {
        line[strlen(line)-1] = '\0';
        if(ddict_add(dict, line, NULL) == 0) {
            n_dict++;
        }
    }

    free(line);
    fclose(fid);
    printf("Added %d words to the dictionary\n", n_dict);
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
                }
            }
        }
    }
    printf("Added %d words to the dictionary\n", n_dict);
    return dict;
}

char * read_and_split(const char * file, size_t * _file_size)
{
    FILE * fid = fopen(file, "r");
    fseek(fid, 0, SEEK_END);
    size_t file_size = ftell(fid);
    fseek(fid, 0, SEEK_SET);
    char * buf = malloc(file_size+1);
    assert(buf != NULL);
    ssize_t nread = fread(buf, 1, file_size, fid);
    assert(nread == (ssize_t) file_size);
    fclose(fid);
    *_file_size = file_size;
    // Replace blacks by '\0'
    for(size_t kk = 0; kk < file_size; kk++) {
        if(buf[kk] == '\n') {
            buf[kk] = '\0';
        }
    }
    buf[file_size] = '\0';
    return buf;
}

int test_dict_keys(const char * dictfile, const char * txtfile, int manage_keys)
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
        dict = dict_from_buffer(key_buffer, key_buffer_size);
        clock_gettime(CLOCK_REALTIME, &t1);
    }

    // 2. Find all words not in the dict from
    // the txtfile
    wreader * reader = word_reader_new(txtfile);
    printf("Looking for unknown words in %s\n", txtfile);
    int n_known = 0;
    int n_unknown = 0;
    char * word;
    while(word_reader_read(reader, &word))
    {
        if(ddict_get(dict, word) != NULL){
            n_known++;
        } else {
            n_unknown++;
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

int main(int argc, char ** argv)
{
    // https://github.com/Torbacka/wordlist
    const char* dictfile  = "ord.txt";
    // https://www.gutenberg.org/cache/epub/75742/pg75742.txt
    const char* txtfile = "pg75742.txt";

    if(argc > 1) {
        dictfile = argv[1];
    }
    if(argc > 2) {
        txtfile = argv[2];
    }

#ifdef DDICT_DROP_HASH
    printf("-> Managed keys & DDICT_DROP_HASH\n");
#else
    printf("-> Managed keys\n");
#endif
    test_dict_keys(dictfile, txtfile, 1);
#ifdef DDICT_DROP_HASH
    printf("\n-> External keys & DDICT_DROP_HASH\n");
#else
    printf("\n-> External keys\n");
#endif
    test_dict_keys(dictfile, txtfile, 0);


    size_t VmPeak, VmHWM;
    if(get_peak_memory_KB(&VmPeak, &VmHWM) == 0){
        printf("\n");
        printf("VmPeak: %zu kb, VmHWM: %zu kb\n", VmPeak, VmHWM);
    }
    return EXIT_SUCCESS;
}
