## ddict

An throwaway implementation of a dict data structure inspired by the approach taken by [Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c) which is described in this [blog post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).

Summary:

- Entries `(char * key, u64 hash, void * value)` are stored in
  insertion order, i.e., it is possible to loop over them in insertion
  order.
- Open Addressing with linear probing.
- Index starts as 8 bit per element and switches to larger element
  sizes as needed.
- Grows dynamically, doubles when 50% of the index capacity is reached.
- Performance in the expected range (see below).
- Keys are either owned by the dictionary or handled externally.
- If `DDICT_DROP_HASH` is defined, the entries will exclude the hash
values and calculate them on the fly. This will reduce the memory
load but cause redundant calculations.
- Can be built with clang/gcc/musl-gcc, gnu99 standard or above
  (`ddict_test.c` requires gnu99 or above).

Limitations/missing features:

- Keys can't be removed once inserted (but the "value" of the entries can be altered).
- Keys are '\0' terminated strings.

## Usage

You just need `ddict.h` and `ddict.c`. For examples, see `ddict_test.c`.

<details><summary>Example output</summary>

```
$ ./ddict_test 0
#
# Example 1 -- basic usage
#
dict_add['username'] = 'john_doe'
dict_add['password'] = '1234'
dict_add['password'] = '4567'
Can not ADD 'password', already set
dict_update['password'] = '4567'
dict_get['username'] -> 'john_doe'
dict_get['password'] -> '4567'
dict_get['height'] -> ERROR: 'height' is not in the dictionary
#
# Example 2 -- looping over entries
#
entry 1/2 key: 'username', value: 'john_doe'
entry 2/2 key: 'password', value: '1234'

VmPeak: 2708 kb, VmHWM: 1556 kb
```

</details>

## Performance indicators (sanity check)

Comparing apples to oranges, really!

<details><summary>Test details</summary>

With Python:

``` Python
    dct = dict()

    for n in range(N):
        dct[f'{n}'] = 0

    for n in range(N):
        assert(dct[f'{n}'] == 0)
```

With ddict:

``` C
    ddict * dict = ddict_new(1);

    for(u64 kk = 0; kk < n; kk++) {
        sprintf(word, "%lu", kk);
        ddict_add(dict, word, 0));
    }

    for(u64 kk = 0; kk < n; kk++)
        assert(ddict_get(dict, word));
```

With go:

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

</details>

Test results (Intel i7-6700K, GCC 13.3.0, Python 3.12.3, go 1.26.5)

| method | N   | t_create [ms] | t_scan [ms] | t_total [ms] | VmHWM [MB] |
|--------|-----|--------------:|------------:|-------------:|-----------:|
| ddict  | 1e5 |            19 |           2 |           21 |          6 |
| go     | 1e5 |            15 |           6 |           22 |          8 |
| python | 1e5 |            20 |          13 |           33 |         20 |
| ddict  | 1e6 |           170 |          14 |          183 |         39 |
| go     | 1e6 |           275 |         156 |          432 |         82 |
| python | 1e6 |           267 |         232 |          500 |         87 |
| ddict  | 1e7 |          1822 |         163 |         1985 |        442 |
| go     | 1e7 |          3535 |        1908 |         5443 |        686 |
| python | 1e7 |          3562 |        2986 |         6548 |        720 |
| ddict  | 1e8 |         19447 |        2419 |        21886 |       5093 |
| go     | 1e8 |         42592 |       30695 |        73287 |       5749 |
| python | 1e8 |         46690 |       20669 |        77359 |      11234 |
|        |     |               |             |              |            |
| cdicts | 1e5 |            59 |          23 |           83 |         31 |
| cdicts | 1e6 |           733 |         345 |         1078 |        291 |
| cdicts | 1e7 |          8268 |        3452 |        11720 |       2946 |
| cdicts | 1e8 |        104044 |       35993 |       140037 |      30224 |
|        |     |               |             |              |            |

## Notes

- It is costly to `realloc` the entries, and even more costly to
  grow the index, since it has to be re-built each time that happens.

- When the key storage is full, it is `realloc`'ed. That is not
  necessary since we could split up the keys and store them in
  multiple buffers.


## Alternatives?

- Write something that you like and want to use.
- [https://gitlab.com/rob.izzard/libcditc]
- [https://github.com/fmela/libdict]
