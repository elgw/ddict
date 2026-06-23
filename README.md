## ddict

An throwaway implementation of a dict data structure inspired by the approach taken by [Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c) which is described in this [blog post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).

- Incomplete functionality for most uses.
- Performance in the expected range (see below).
- All keys are either owned by the dictionary or handled externally.
- Optional to store the hash values or calculate them on the fly when needed (define `DDICT_DROP_HASH`).
- A "safer" version should use randomized hash functions for security
  reasons.

## Timings
In this case we create a dictionary, where each line in the file
`ord.txt` is added as a key:

```
$ wc ord.txt
 124761  125031 1440709 ord.txt
$ uniq ord.txt  | wc
 122475
$ stress -c 8 -t 8 ; ./test_ddict
```

Each word in

```
$ wc pg75742.txt
  2142  18350 124696 pg75742.txt
```

is scanned against the dictionary to identify which words were not in
the dictionary file.

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

Vs, Python 3.12.3, where the timings might be dominated by other
factors besides the actual dict implementation.

```
$ stress -c 8 -t 8 ; ./pydict_test.py
dded 122475 words to the dictionary
Found 9070 known and 9280 unknown words
Construct: 34.481 ms
Scan: 5.959 ms
Total: 40.440 ms
```

Vs go 1.26.4

```
$ cd testgo
$ go build
$ stress -c 8 -t 8 ; ./dict
Construction took 28.234787ms
122475  words added to the dictionary
Scanning took 1.806676ms
n_words  18350
Known:  9070  unknown:  9280
Total time: 30.041463ms
```

## TODO

- [ ] Unit tests
- [ ] update by key
- [ ] remove by key
- [ ] serialize and de-serialize
- [ ] more efficient key storage
- [ ] comparisons might be more fair with one word per line?
