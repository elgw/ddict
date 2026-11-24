## ddict 1.0.2

An throwaway implementation of a dict data structure inspired by the
approach taken by
[Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c)
which is described in this [blog
post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).


### Summary

- Entries `(char * key, u64 hash, void * value)` be accessed directly
  by their insertion order, i.e., can be looped over.
- Keys are either owned by the dictionary or handled externally.
- If `DDICT_DROP_HASH` is defined, the entries will exclude the hash
values and calculate them on the fly. This will reduce the memory
load but cause redundant calculations.
- Can be built with clang/gcc/musl-gcc, c99 standard or above
  (`ddict_test.c` requires gnu99 or above).
- After compilation, `ddict.o` is approximately 5 kB
  (`libglib-2.0.so.0.8000.0` is 1.3 MB, `libcdict.so` is 2.2M, however,
  those are more feature rich).
- Performance in the expected range (see below).
- Approx 400 SLOC so it should not be too complicate to adjust the
  code for other use cases.

Limitations/missing features

- Keys can't be removed once inserted (but the "value" of the entries
  can be altered).
- Keys are `'\0'` terminated strings.

Internals

- Open Addressing with linear probing.
- Index starts as 8 bit per element and switches to larger element
  sizes as needed.

## Usage

You just need `ddict.h` and `ddict.c`. For example usage, see `ddict_test.c`.

## API
See `ddict.h`
``` C
ddict * ddict_new(int manage_keys);
void ddict_free(ddict * dict);
ddict_entry * ddict_get(const ddict *, const char * key);
int ddict_add(ddict * dict, char * key, void * value);
int ddict_update_entry(ddict * dict, const char * key, void * value);
```

## Performance indicators (sanity check)

Comparing apples to oranges, really!

<details><summary>Test details</summary>

We use Python as the base case, and express the test simply as:

``` Python
    dct = dict()

    for n in range(N):
        dct[f'{n}'] = 0

    for n in range(N):
        assert(dct[f'{n}'] == 0)
```

In this case the keys are owned by the dict/runtime.

Similarly, With `ddict` we let the dictionary own the keys via the
argument `manage_keys = 1`.

``` C
    ddict * dict = ddict_new(1); // the dictionary owns copies of the keys

    for(u64 kk = 0; kk < n; kk++) {
        sprintf(word, "%lu", kk);
        ddict_add(dict, word, 0));
    }
    for(u64 kk = 0; kk < n; kk++)
    {
        sprintf(word, "%lu", kk);
        if( ddict_get(dict, word) == NULL) {
            printf("Failed to retrieve '%s'\n", word);
            exit(EXIT_FAILURE);
        }
    }
```

With **glib** we use the same hash function as we did with `ddict`:

``` C
    GHashTable* H = g_hash_table_new (_ddict_wordhash,
                                      g_str_equal);
```

Since the hash table can't own the keys, we write them to a dense
buffer before inserting:

``` C
for(uint64_t n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        g_hash_table_insert(H, key_write, NULL);
        key_write += nwritten + 1;
    }
```

key strings are re-generated while searching (for fairness, the string
generation takes a considerable time).

``` C
for(u64 n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        if(g_hash_table_contains(H, key_write) == 0)
        {
            printf("Failure\n");
            exit(EXIT_FAILURE);
        };
        key_write += nwritten + 1;
    }
```

I don't know `golang`, but this seems similar enough:

``` go
var m = make(map[string]int)

	for i := 0; i < N; i++ {
		m[strconv.Itoa(i)] = 0
	}

    for i := 0; i < N; i++ {
		_, ok = m[strconv.Itoa(i)]
		if ok == false { panic("panic!"); }
	}
```

With [cdict.h](https://github.com/RobusGauli/cdict.h) I used:

``` C
// initialization
typedef char* string;
CDict(string, int) cdict_t;
cdict_t cdict_instance;
cdict__init(&cdict_instance);

// ...

// Add elements
    for(u64 n = 0; n < N; n++)
    {
        u64 nwritten = sprintf(key_write, "%lu", n);
        cdict__add(&cdict_instance, key_write, 0);
        key_write += nwritten + 1;
    }

// ...

// Retrieval
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

// ...

// Cleanup
cdict__clear(&cdict_instance);
cdict__free(&cdict_instance);
```

With [libcdict 1.51](https://gitlab.com/rob.izzard/libcdict) I used:

``` C
CDict_new(c);
# ...
for(uint64_t n = 0; n < N; n++)
    {
        sprintf(word, "%lu", n);
        CDict_set_with_types(c, word, CDICT_DATA_TYPE_STRING,
                             0, CDICT_DATA_TYPE_INT);
    }
# ...
for(uint64_t n = 0; n < N; n++)
    {
        sprintf(word, "%lu", n);
        if(CDict_nest_get_entry(c, word) == NULL)
        {
            printf("Failed to retrieve '%s'\n", word);
            exit(EXIT_FAILURE);
        }
    }
    CDict_free_and_free_contents(c);
```

</details>

Test were run with an Intel i7-6700K, GCC 13.3.0, [glib 2.39](https://docs.gtk.org/glib/), Python
3.12.3, go 1.26.5, [libcdict
1.51](https://gitlab.com/rob.izzard/libcdict), [cdict.h
afefd33](https://github.com/RobusGauli/cdict.h)). C-programs were
compiled with `-O3 -ffast-math -DNDEBUG`
and linked with `-flto`.

Results:

| method   |     N | t_create [ms] | t_scan [ms] | t_total [ms] | VmHWM [MB] |
|----------|------:|--------------:|------------:|-------------:|-----------:|
| ddict    | 1e+05 |            11 |           6 |       **17** |          6 |
| ddict    | 1e+06 |           125 |          61 |      **187** |         55 |
| ddict    | 1e+07 |         1,201 |         718 |    **1,919** |        442 |
| ddict    | 1e+08 |        13,764 |       6,606 |   **20,370** |      5,093 |
|          |       |               |             |              |            |
| glib     | 1e+05 |            12 |           7 |           19 |      **5** |
| glib     | 1e+06 |           180 |          70 |          251 |     **42** |
| glib     | 1e+07 |         1,396 |         795 |        2,191 |    **342** |
| glib     | 1e+08 |        23,265 |       9,668 |       32,933 |  **2,968** |
|          |       |               |             |              |            |
| cdict.h  | 1e+05 |            18 |           7 |           25 |         11 |
| cdict.h  | 1e+06 |           254 |         151 |          405 |         80 |
| cdict.h  | 1e+07 |         2,948 |       1,767 |        4,715 |        636 |
| cdict.h  | 1e+08 |        42,347 |      22,764 |       65,111 |     10,254 |
|          |       |               |             |              |            |
| go       | 1e+05 |            11 |           5 |       **17** |          8 |
| go       | 1e+06 |           269 |         152 |          424 |         81 |
| go       | 1e+07 |         3,537 |       1,906 |        5,444 |        688 |
| go       | 1e+08 |        42,866 |      30,327 |       73,194 |      5,853 |
|          |       |               |             |              |            |
| Python   | 1e+05 |            19 |          12 |           31 |         20 |
| Python   | 1e+06 |           261 |         225 |          486 |         88 |
| Python   | 1e+07 |         3,487 |       2,997 |        6,484 |        721 |
| Python   | 1e+08 |        46,205 |      29,606 |       75,811 |     11,235 |
|          |       |               |             |              |            |
| libcdict | 1e+05 |            58 |          31 |           88 |         31 |
| libcdict | 1e+06 |           719 |         449 |        1,171 |        292 |
| libcdict | 1e+07 |         8,336 |       4,943 |       13,279 |      2,946 |
| libcdict | 1e+08 |       104,093 |      52,387 |      156,481 |     30,224 |


<details><summary>AMD 3700X</summary>

| method   |     N | t_create [ms] | t_scan [ms] | t_total [ms] | VmHWM [MB] |
|----------|------:|--------------:|------------:|-------------:|-----------:|
| ddict    | 1e+05 |            11 |           6 |           18 |          6 |
| ddict    | 1e+06 |           124 |          67 |          191 |         55 |
| ddict    | 1e+07 |         1,244 |         733 |        1,980 |        442 |
| ddict    | 1e+08 |        12,487 |       6,108 |       18,595 |      5,092 |
|          |       |               |             |              |            |
| glib     | 1e+05 |            14 |           8 |           22 |          5 |
| glib     | 1e+06 |           211 |          81 |          296 |         42 |
| glib     | 1e+07 |         1,757 |       1,023 |        2,780 |        342 |
| glib     | 1e+08 |        26,086 |      11,267 |       37,354 |      2,967 |
|          |       |               |             |              |            |
| cdict.h  | 1e+05 |            18 |           7 |           25 |         11 |
| cdict.h  | 1e+06 |           268 |         172 |          440 |         80 |
| cdict.h  | 1e+07 |         3,198 |       2,032 |        5,230 |        636 |
| cdict.h  | 1e+08 |        45,323 |      24,404 |       69,727 |     10,253 |
|          |       |               |             |              |            |
| go       | 1e+05 |            13 |           4 |           18 |          9 |
| go       | 1e+06 |           289 |         135 |          425 |         82 |
| go       | 1e+07 |         4,056 |       1,772 |        5,829 |        698 |
| go       | 1e+08 |        47,448 |      30,199 |       77,647 |      5,835 |
|          |       |               |             |              |            |
| Python   | 1e+05 |            18 |          13 |           31 |         20 |
| Python   | 1e+06 |           273 |         234 |          510 |         89 |
| Python   | 1e+07 |         3,908 |       3,537 |        7,446 |        721 |
| Python   | 1e+08 |        50,011 |      34,316 |       84,326 |     11,235 |
|          |       |               |             |              |            |
| libcdict | 1e+05 |            67 |          37 |          104 |         31 |
| libcdict | 1e+06 |           727 |         502 |        1,229 |        292 |
| libcdict | 1e+07 |         7,995 |       6,061 |       14,056 |      2,945 |
| libcdict | 1e+08 |        97,538 |      63,896 |      161,434 |     30,224 |


</details>

<details><summary>Raspberry Pi 4 Model B Rev 1.5, 4 GB RAM, Debian 12</summary>

| method   |     N | t_create [ms] | t_scan [ms] | t_total [ms] | VmHWM [MB] |
|----------|------:|--------------:|------------:|-------------:|-----------:|
| ddict    | 1e+05 |            39 |          26 |           65 |          6 |
| ddict    | 1e+06 |           417 |         257 |          675 |         55 |
| ddict    | 1e+07 |         4,081 |       2,888 |        6,968 |        442 |
|          |       |               |             |              |            |
| glib     | 1e+05 |            52 |          34 |           87 |          5 |
| glib     | 1e+06 |           993 |         324 |        1,317 |         42 |
| glib     | 1e+07 |         6,221 |       3,286 |        9,506 |        341 |
|          |       |               |             |              |            |
| cdict.h  | 1e+05 |            86 |          42 |          127 |         11 |
| cdict.h  | 1e+06 |           903 |         469 |        1,375 |         80 |
| cdict.h  | 1e+07 |         9,745 |       5,881 |       15,626 |        636 |
|          |       |               |             |              |            |
| Python   | 1e+05 |            72 |          49 |          120 |         20 |
| Python   | 1e+06 |           894 |         640 |        1,534 |        101 |
| Python   | 1e+07 |        10,947 |       8,392 |       19,339 |        876 |
|          |       |               |             |              |            |
| libcdict | 1e+05 |           244 |         150 |          394 |         31 |
| libcdict | 1e+06 |         2,498 |       1,678 |        4,176 |        291 |
| libcdict | 1e+07 |        26,999 |      17,305 |       44,304 |      2,945 |

</details>

<details><summary>AMD Ryzen 5 5600</summary>

| method   |     N | t_create [ms] | t_scan [ms] | t_total [ms] | VmHWM [MB] |
|----------|------:|--------------:|------------:|-------------:|-----------:|
| ddict    | 1e+05 |             8 |           4 |           12 |          6 |
| ddict    | 1e+06 |            89 |          43 |          132 |         55 |
| ddict    | 1e+07 |           853 |         523 |        1,376 |        442 |
| ddict    | 1e+08 |         9,670 |       4,552 |       14,222 |      5,093 |
|          |       |               |             |              |            |
| glib     | 1e+05 |             9 |           5 |           14 |          5 |
| glib     | 1e+06 |           147 |          48 |          195 |         42 |
| glib     | 1e+07 |         1,269 |         668 |        1,942 |        341 |
| glib     | 1e+08 |        21,729 |       8,441 |       30,169 |      2,968 |
|          |       |               |             |              |            |
| cdict.h  | 1e+05 |            14 |           6 |           19 |         11 |
| cdict.h  | 1e+06 |           179 |         109 |          290 |         80 |
| cdict.h  | 1e+07 |         2,902 |       1,882 |        4,784 |        636 |
| cdict.h  | 1e+08 |        39,001 |      18,728 |       57,729 |     10,254 |
|          |       |               |             |              |            |
| go       | 1e+05 |            11 |           4 |           16 |          8 |
| go       | 1e+06 |           204 |         102 |          307 |         80 |
| go       | 1e+07 |         3,663 |       1,820 |        5,484 |        684 |
| go       | 1e+08 |        46,246 |      23,259 |       69,506 |      5,830 |
|          |       |               |             |              |            |
| Python   | 1e+05 |            14 |           9 |           23 |         19 |
| Python   | 1e+06 |           184 |         136 |          322 |         87 |
| Python   | 1e+07 |         3,386 |       3,016 |        6,402 |        720 |
| Python   | 1e+08 |        45,732 |      28,710 |       74,441 |     11,234 |
|          |       |               |             |              |            |
| libcdict | 1e+05 |            40 |          19 |           60 |         31 |
| libcdict | 1e+06 |           547 |         341 |          891 |        292 |
| libcdict | 1e+07 |         6,533 |       4,522 |       11,056 |      2,946 |
| libcdict | 1e+08 |        78,467 |      49,241 |      127,708 |     30,224 |
|          |       |               |             |              |            |

</details>

### Notes

- The memory usage of `ddict` can be reduced as the cost of speed, see
  the parameters close to the top of `ddict.c`.

- It is costly to `realloc` the entries, and even more costly to
  grow the index, since it has to be re-built each time that happens.

- In the tests, a fixed buffer is used to store the keys for `glib`, `cdict.h` and
  `libcdict` while `ddict` manage copies of the keys
  internally. When the key storage is full, it is `realloc`'ed. That is not
  necessary since we could split up the keys and store them in
  multiple buffers.  `ddict` runs slightly faster when the keys are kept
  in a fixed buffer (about 7%, not shown in the table(s)).
