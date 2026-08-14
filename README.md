## ddict 1.0.1

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
- Can be built with clang/gcc/musl-gcc, gnu99 standard or above
  (`ddict_test.c` requires gnu99 or above).
- After compilation, `ddict.o` is approximately 5 kB
  (`libglib-2.0.so.0.8000.0` is 1.3 MB, `libcdict.so` is 2.2M, but
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
    ddict * dict = ddict_new(1);

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

With `cdicts` I used:

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

Test were run with an Intel i7-6700K, GCC 13.3.0, glib 2.39, Python 3.12.3, go
1.26.5, [libcdict 1.51](https://gitlab.com/rob.izzard/libcdict)). C-programs were
compiled with `-O3 -ffast-math -DNDEBUG -march=native -mtune=native`
and linked with `-flto`.

Results:

| method | N   | t_create [ms] | t_scan [ms] | t_total [ms] | VmHWM [MB] |
|--------|-----|--------------:|------------:|-------------:|-----------:|
| ddict  | 1e5 |            18 |           6 |           25 |          6 |
| ddict  | 1e6 |           146 |          73 |      **219** |         46 |
| ddict  | 1e7 |         1,359 |         672 |    **2,031** |        442 |
| ddict  | 1e8 |        18,808 |       6,280 |   **25,088** |      5,093 |
|        |     |               |             |              |            |
| glib   | 1e5 |            13 |           7 |       **20** |      **5** |
| glib   | 1e6 |           183 |          66 |          248 |     **42** |
| glib   | 1e7 |         1,409 |         748 |        2,156 |    **341** |
| glib   | 1e8 |        23,292 |       9,203 |       32,346 |  **2,967** |
|        |     |               |             |              |            |
| go     | 1e5 |            15 |           6 |           22 |          8 |
| go     | 1e6 |           275 |         156 |          432 |         82 |
| go     | 1e7 |         3,535 |       1,908 |        5,443 |        686 |
| go     | 1e8 |        42,592 |      30,695 |       73,287 |      5,749 |
|        |     |               |             |              |            |
| python | 1e5 |            20 |          13 |           33 |         20 |
| python | 1e6 |           267 |         232 |          500 |         87 |
| python | 1e7 |         3,562 |       2,986 |        6,548 |        720 |
| python | 1e8 |        46,690 |      20,669 |       77,359 |     11,234 |
|        |     |               |             |              |            |
| cdicts | 1e5 |            59 |          23 |           83 |         31 |
| cdicts | 1e6 |           733 |         345 |        1,078 |        291 |
| cdicts | 1e7 |         8,268 |       3,452 |       11,720 |      2,946 |
| cdicts | 1e8 |       104,044 |      35,993 |      140,037 |     30,224 |
|        |     |               |             |              |            |

### Notes

- `VmHWM` is what it is.

- The memory usage of `ddict` can be reduced as the cost of speed, see
  the parameters close to the top of `ddict.c`.

- It is costly to `realloc` the entries, and even more costly to
  grow the index, since it has to be re-built each time that happens.

- When the key storage is full, it is `realloc`'ed. That is not
  necessary since we could split up the keys and store them in
  multiple buffers. The glib benchmark does not suffer from this since
  all keys are written to a fixed buffer.

- For this test, glib could have used the address as the hash function
  which would be even faster.
