## ddict

An throwaway implementation of a dict data structure inspired by the approach taken by [Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c) which is described in this [blog post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).

- Performance in the expected range (see below).
- All keys are either owned by the dictionary or handled externally.
- Optional to store the hash values or calculate them on the fly when needed (define `DDICT_DROP_HASH`).
- Can be built with clang/gcc/musl-gcc, C99 standard or above.

## Timings
A file with one word per line is used to construct a dictionary, then
each word in another text file is scanned against the dictionary. By
default the test programs assumes that these are called
`dictwords.txt` and `text.txt`.

<details><summary>Get the input files used for the test below</summary>

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

| method              | t_construct [ms] | t_scan [ms] |
|---------------------|------------------|-------------|
| ddict-external      | 9.4              | 1.3         |
| ddict-external-drop | 11.8             | 1.5         |
| ddict-managed       | 17.6             | 1.4         |
| ddict-managed-drop  | 20.0             | 1.6         |
| Go-1.26.4           | 28.2             | 1.8         |
| Python-3.12.3       | 35.5             | 6.0         |




<details>
<summary>Command line output from the benchmarks</summary>

Please note that the timings might be dominated by other factors
besides the actual dict implementation, e.g. file parsing,
encapsulation, garbage collection etc.

```
-> Managed keys
Reading lines from ord.txt
Added 122475 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9070 known and 9280 unknown words
Construct: 17.549 ms
Scan: 1.352 ms
Total: 18.902 ms

-> External keys
Added 122475 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9070 known and 9280 unknown words
Construct: 9.539 ms
Scan: 1.305 ms
Total: 10.844 ms

-> Managed keys & DDICT_DROP_HASH
Reading lines from ord.txt
Added 122475 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9070 known and 9280 unknown words
Construct: 19.926 ms
Scan: 1.607 ms
Total: 21.533 ms

-> External keys & DDICT_DROP_HASH
Added 122475 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9070 known and 9280 unknown words
Construct: 11.751 ms
Scan: 1.465 ms
Total: 13.215 ms
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
```
</details>

## TODO

- [ ] Unit tests
- [ ] update by key
- [ ] remove by key
- [ ] serialize and de-serialize
