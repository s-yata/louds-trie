# louds-trie

LOUDS-trie implementation example (C++)

## Results

```
$ export LC_ALL=C

$ ls -sh1 data/
total 344M
303M enwiki-20191001-all-titles-in-ns0
 41M jawiki-20191001-all-titles-in-ns0

$ sort data/jawiki-20191001-all-titles-in-ns0 > data/jawiki-20191001-all-titles-in-ns0.sorted
$ sort data/enwiki-20191001-all-titles-in-ns0 > data/enwiki-20191001-all-titles-in-ns0.sorted

$ make
g++ -O2 -Wall -Wextra -march=native *.cpp -o louds-trie

$ ./louds-trie < data/jawiki-20191001-all-titles-in-ns0.sorted
build = 135.789 ns/key
#keys = 1887667
#nodes = 18970273
size = 28208993 bytes
seq. lookup = 460.763 ns/key
rnd. lookup = 1398.13 ns/key

$ ./louds-trie < data/enwiki-20191001-all-titles-in-ns0.sorted
build = 123.173 ns/key
#keys = 14837096
#nodes = 130740728
size = 194319332 bytes
seq. lookup = 438.351 ns/key
rnd. lookup = 2573.98 ns/key
```

## See also

https://github.com/s-yata/louds-patricia
