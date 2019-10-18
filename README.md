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
build = 144.602 ns/key
#keys = 1887667
#nodes = 18970273
size = 27581969 bytes
seq. lookup = 508.046 ns/key
rnd. lookup = 1471.03 ns/key

$ ./louds-trie < data/enwiki-20191001-all-titles-in-ns0.sorted
build = 120.86 ns/key
#keys = 14837096
#nodes = 130740728
size = 189997232 bytes
seq. lookup = 505.853 ns/key
rnd. lookup = 2840.81 ns/key
```
