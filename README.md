## ddict

An throwaway implementation of a dict data structure inspired by the approach taken by [Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c) which is described in this [blog post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).

- Incomplete functionality for most uses.
- Performance in the expected range (see below).
- All keys are either owned by the dictionary or handled externally.
- Optional to store the hash values or calculate them on the fly when needed (define `DDICT_DROP_HASH`).
- A "safer" version should use randomized hash functions for security
  reasons.

## Timings

```
$ stress -c 8 -t 8 ; ./test_ddict
```

```
-> Managed keys
Reading words from ord.txt
Added 122446 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9157 known and 9193 unknown words
Construct: 21.351 ms
Scan: 1.486 ms
Total: 22.837 ms

-> External keys
Allocated 1440709 bytes
Looking for unknown words in pg75742.txt
Found 9157 known and 9193 unknown words
Construct: 13.232 ms
Scan: 1.667 ms
Total: 14.899 ms

-> Managed keys & DDICT_DROP_HASH
Reading words from ord.txt
Added 122446 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9157 known and 9193 unknown words
Construct: 23.988 ms
Scan: 2.713 ms
Total: 26.701 ms

-> External keys & DDICT_DROP_HASH
Added 125030 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9157 known and 9193 unknown words
Construct: 17.223 ms
Scan: 1.571 ms
Total: 18.794 ms
```

Vs, Python 3.12.3, where the timings might be dominated by other
factors besides the actual dict implementation.

```
$ stress -c 8 -t 8 ; ./pydict_test.py
Added 122446 words to the dictionary
Found 9157 known and 9193 unknown words
Construct: 46.872 ms
Scan: 5.976 ms
Total: 52.847 ms
```

Vs go 1.26.4

```
$  cd testgo
$ go build
$ stress -c 8 -t 8 ; ./dict
Construction took 36.600434ms
122446  words added to the dictionary
Scanning took 1.775793ms
n_words  18350
Known:  9157  unknown:  9193
Total time: 38.376227ms
```

## TODO

- [ ] Unit tests
- [ ] update by key
- [ ] remove by key
- [ ] serialize and de-serialize
- [ ] more efficient key storage
- [ ] comparisons might be more fair with one word per line?
