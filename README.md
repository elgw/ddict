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
```

Next steps outlined in the source and header files, but makes sense to
fix the word parsing first. Then look at the silly amount of malloc/free...
