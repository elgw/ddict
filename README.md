## ddict

An throwaway implementation of a dict data structure inspired by the approach taken by [Python](https://github.com/python/cpython/blob/main/Objects/dictobject.c) which is described in this [blog post](https://morepypy.blogspot.com/2015/01/faster-more-memory-efficient-and-more.html).

- Incomplete functionality for most uses.
- Performance in the expected range (see below).
- All keys are owned by the dictionary which seems reasonable unless a
  reference counting system is used.
- Store hash for faster comparisons, but that can of course be removed
  to save some memory.
- A "safer" version should use randomized hash functions for security
  reasons.

## Timings

```
$ stress -c 8 -t 8 ; ./test_ddict
stress: info: [113950] dispatching hogs: 8 cpu, 0 io, 0 vm, 0 hdd
stress: info: [113950] successful run completed in 8s
Reading words from ord.txt
Added 122446 words to the dictionary
Looking for unknown words in pg75742.txt
Found 9157 known and 9193 unknown words
Construct: 0.018
Scan: 0.001
Total: 0.020
```

Vs, python (where the timings might be dominated by other things than the actual
dict implementation).
```
$ stress -c 8 -t 8 ; ./pydict_test.py
stress: info: [114014] dispatching hogs: 8 cpu, 0 io, 0 vm, 0 hdd
stress: info: [114014] successful run completed in 8s
Added 122446 words to the dictionary
Found 9157 known and 9193 unknown words
Construct: 0.048
Scan: 0.005
Total: 0.053
```
