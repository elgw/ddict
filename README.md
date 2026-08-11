## ddict

An throwaway implementation of a dict data structure inspired by the approach taken by [Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c) which is described in this [blog post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).

Summary:
- Entries `(char * key, u64 hash, void * value)` are stored in insertion order.
- Open Addressing with linear probing.
- Grows dynamically, doubles when 50% of the capacity is reached.
- Performance in the expected range (see below).
- Keys are either owned by the dictionary or handled externally.
- If `DDICT_DROP_HASH` is defined, the entries will exclude the hash
values and calculate them on the fly. This will reduce the memory
load but cause redundant calculations.
- Can be built with clang/gcc/musl-gcc, C99 standard or above.

## Usage

You just need `ddict.h` and `ddict.c`. For examples, see `ddict_test.c`.

## Performance indicators (sanity check)

A file with one word per line is used to construct a dictionary, then
each word in another text file is scanned against the dictionary. By
default the test programs assumes that these are called
`dictwords.txt` and `text.txt`.

The timings might be dominated by other factors
besides the actual dict implementation, e.g. file parsing,
encapsulation, garbage collection etc.

| method                 | t_construct [ms] | t_scan [ms] | t_total [ms] | VmHWM [kb] |
|------------------------|------------------|-------------|--------------|------------|
| ddict:external,cached  | 11.1             | 1.3         | 12.4         | 6884       |
| ddict:external,dropped | 14.6             | 1.5         | 16.1         | 6928       |
| ddict:managed,cached   | 15.7             | 1.3         | 17.0         | 9240       |
| ddict:managed,dropped  | 23.2             | 1.5         | 24.6         | 9240       |
| Go-1.26.4              | 28.2             | 1.8         | 30           | 14948      |
| Python-3.12.3          | 33.4             | 5.7         | 39.1         | 23088      |
|                        |                  |             |              |            |


<details><summary>Get the input files used for the timings</summary>

- `dictwords.txt`

```
$ wget https://raw.githubusercontent.com/Torbacka/wordlist/refs/heads/master/saol2018clean.csv
$ awk -F, '{print $2}' saol2018clean.csv > dictwords.txt
$ wc dictwords.txt
 124761  125031 1440709 dictwords.txt
$ uniq dictwords.txt  | wc
 122475
```

- `text.txt`


```
$ wget https://www.gutenberg.org/ebooks/75742.txt.utf-8
$ wc 75742.txt.utf-8
  2142  18350 124696 75742.txt.utf-8
$ mv 75742.txt.utf-8 text.txt
```
</details>


<details>
<summary>Example command line output</summary>


```
$ make
$ ./ddict_test 2
-> External keys, hash values cached
Added 122475 words to the dictionary
Looking for unknown words in text.txt
Found 9070 known and 9280 unknown words
Construct: 12.529 ms
Scan: 1.439 ms
Total: 13.967 ms
VmPeak: 8372 kb, VmHWM: 6924 kb
```

Python 3.12.3

```
$ stress -c 8 -t 8 ; python test/pydict.py
dded 122475 words to the dictionary
Found 9070 known and 9280 unknown words
Construct: 34.481 ms
Scan: 5.959 ms
Total: 40.440 ms
```

Go 1.26.4

```
$ cd test/go/
$ go build
$ stress -c 8 -t 8 ; ./dict
Construction took 28.234787ms
122475  words added to the dictionary
Scanning took 1.806676ms
n_words  18350
Known:  9070  unknown:  9280
Total time: 30.041463ms
VmHWM:	   14948 kB
```
</details>

## TODO?

- [ ] Unit tests
- [ ] update by key
- [ ] remove by key
- [ ] serialize and de-serialize
