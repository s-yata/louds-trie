#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

#include "louds-trie.hpp"

using namespace std;
using namespace std::chrono;

int main() {
  ios_base::sync_with_stdio(false);
  vector<string> keys;
  string line;
  while (getline(cin, line)) {
    keys.push_back(line);
  }

  high_resolution_clock::time_point begin = high_resolution_clock::now();
  louds::Trie trie;
  for (uint64_t i = 0; i < keys.size(); ++i) {
    trie.add(keys[i]);
  }
  trie.build();
  high_resolution_clock::time_point end = high_resolution_clock::now();
  double elapsed = (double)duration_cast<nanoseconds>(end - begin).count();
  cout << "build = " << (elapsed / keys.size()) << " ns/key" << endl;

  cout << "#keys = " << trie.n_keys() << endl;
  cout << "#nodes = " << trie.n_nodes() << endl;
  cout << "size = " << trie.size() << " bytes" << endl;

  begin = high_resolution_clock::now();
  for (uint64_t i = 0; i < keys.size(); ++i) {
    assert(trie.lookup(keys[i]) != -1);
  }
  end = high_resolution_clock::now();
  elapsed = (double)duration_cast<nanoseconds>(end - begin).count();
  cout << "seq. lookup = " << (elapsed / keys.size()) << " ns/key" << endl;

  random_shuffle(keys.begin(), keys.end());

  begin = high_resolution_clock::now();
  for (uint64_t i = 0; i < keys.size(); ++i) {
    assert(trie.lookup(keys[i]) != -1);
  }
  end = high_resolution_clock::now();
  elapsed = (double)duration_cast<nanoseconds>(end - begin).count();
  cout << "rnd. lookup = " << (elapsed / keys.size()) << " ns/key" << endl;

  return 0;
}