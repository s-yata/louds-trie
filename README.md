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
build = 142.706 ns/key
#keys = 1887667
#nodes = 18970273
size = 28447025 bytes
seq. lookup = 500.309 ns/key
rnd. lookup = 1504.83 ns/key

$ ./louds-trie < data/enwiki-20191001-all-titles-in-ns0.sorted
build = 121.861 ns/key
#keys = 14837096
#nodes = 130740728
size = 195899028 bytes
seq. lookup = 499.932 ns/key
rnd. lookup = 2802.41 ns/key
```
