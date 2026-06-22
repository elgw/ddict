Most basic dict implementation, performance in expected range:

``` shell
$ python test.py
Found 9157 known and 9193 unknown words
Construct: 0.057
Scan: 0.006
Total: 0.063

$ ./test
Reading words from ord.txt
Added 122446 words to the dictionary
Looking for unknown words in pg75742.txt
Found 8615 known and 11615 unknown words
Construct: 0.090
Scan: 0.016
Total: 0.106

# Version 2, with ddict:
$ ./test
Reading words from ord.txt
Added 122446 words to the dictionary
Looking for unknown words in pg75742.txt
Found 8615 known and 11615 unknown words
Construct: 0.054
Scan: 0.007
Total: 0.061

```

Next steps outlined in the source and header files, but makes sense to
fix the word parsing first. Then look at the silly amount of malloc/free...


See a discussion of Pythons current approach:  [https://stackoverflow.com/questions/327311/how-are-pythons-built-in-dictionaries-implemented/44509302#44509302]

- Two step lookup (might induce two cache misses, but more compact so probably not for small datasets)
- Store hash for faster comparisons
- Randomized hash function for security

See the actual Python implementation: [https://github.com/python/cpython/blob/main/Objects/dictobject.c]

The dense approach is quite aligned with my mindset, I'll go for that!
