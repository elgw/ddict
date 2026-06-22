#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>

#include "ddict.h"

static double timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
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

int main(int argc, char ** argv)
{
    struct timespec t0, t1, t2;

    // 1. Create a dictionary from the dictfile

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

    ddict * dict = ddict_new();
    wreader * reader = word_reader_new(dictfile);
    clock_gettime(CLOCK_REALTIME, &t0);
    printf("Reading words from %s\n", dictfile);
    int n_dict = 0;
    char * word = NULL;
    while(word_reader_read(reader, &word)) {
        if(ddict_add(dict, word, NULL) == 0) {
            n_dict++;
        }
    }

    clock_gettime(CLOCK_REALTIME, &t1);
    word_reader_free(reader);

    printf("Added %d words to the dictionary\n", n_dict);

    // 2. Find all words not in the dict from
    // the txtfile
    reader = word_reader_new(txtfile);
    printf("Looking for unknown words in %s\n", txtfile);
    int n_known = 0;
    int n_unknown = 0;
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
    printf("Found %d known and %d unknown words\n", n_known, n_unknown);

    printf("Construct: %.3f\n", timespec_diff(&t1, &t0));
    printf("Scan: %.3f\n", timespec_diff(&t2, &t1));
    printf("Total: %.3f\n", timespec_diff(&t2, &t0));
    return EXIT_SUCCESS;
}
