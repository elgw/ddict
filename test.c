#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>
#include <locale.h> // for isprint

// locale -a
// sudo locale-gen sv_SE.UTF-8


#include "dicts.h"

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

int isprintsv(const uint8_t c)
{
    if(isspace(c)){
        return 0;
    }
    //printf("c=%d (0xC3=%d) ", c, 0xC3);
    if(c == 0xC3) return 2; // Next byte should be accepted as well
    return isprint(c);
}

int
word_reader_read(wreader * reader, char ** result)
{
    // better to read a multibyte character at a time
    // getchar_u(fid, &bytes, &nbytes);
    i64 pos = ftell(reader->fid);
    int c;

    // 0 -- no printable character found (yet)
    // 1 -- in region with printable characters
    int wpos = 0;

    while(1){
        if(wpos + 1 == reader->buf_size)
        {
            fprintf(stderr, "Out of buffer\n");
            exit(EXIT_FAILURE);
        }
        c = fgetc(reader->fid);

        if(c == EOF){
            return 0;
        }

        pos++;
        if(wpos == 0) {
            int nb = isprintsv(c);
            reader->word[wpos++] = c;

            if(nb == 2) {
                c = fgetc(reader->fid);
                reader->word[wpos++] = c;
            }
            continue;
        }

        int nb = isprintsv(c);
        if(nb > 0) {
            reader->word[wpos++] = c;
            if(nb == 2) {
                c = fgetc(reader->fid);
                reader->word[wpos++] = c;
            }
            continue;
        }

        reader->word[wpos++] = '\0';
        result[0] = reader->word;
        return 1;
    }
    assert(0);
    return 0;
}

int main(int argc, char ** argv)
{
    char * loc = setlocale(LC_ALL, "sv_SE.UTF-8");
    printf("Locale: %s\n", loc);
    // 1. Create a dictionary from
    const char* dictfile  = "/home/erikw/code/Torbacka/wordlist/ord.txt";
    const char* txtfile = "pg75742.txt";

    if(argc > 1) {
        dictfile = argv[1];
    }

    spdict * dict = spdict_new();

    wreader * reader = word_reader_new(dictfile);

    printf("Reading words from %s\n", dictfile);
    int n_dict = 0;
    char * word = NULL;
    while(word_reader_read(reader, &word))
    {
        //printf("%s\n", word);

        i64 *value;
        if(spdict_get(dict, word, (void**) &value)){
            *value++;
        } else {
            spdict_add(dict, word, NULL);
            n_dict++;
        }

    }
    word_reader_free(reader);
    printf("Added %d words to the dictionary\n", n_dict);


    // 2.
    // Find all words not in the dict from
    // https://www.gutenberg.org/cache/epub/75742/pg75742.txt
    reader = word_reader_new(txtfile);
    printf("Looking for unknown words in %s\n", txtfile);
    int n_known = 0;
    int n_unknown = 0;
    while(word_reader_read(reader, &word))
    {

        i64 *value;
        if(spdict_get(dict, word, (void**) &value)){
            //printf(" Known\n");
            n_known++;
        } else {
            //spdict_add(dict, word, NULL);
            //printf("%s\n", word);
            n_unknown++;
            //printf(" Unknown\n");
        }

    }
    word_reader_free(reader);


    // 3. Count the word frequency and print the 10 most common

    // done
    spdict_free(dict);
    printf("Found %d known and %d unknown words\n", n_known, n_unknown);
}
