#ifndef LOUDS_TRIE_HPP
#define LOUDS_TRIE_HPP

#include <cstdint>
#include <string>

namespace louds {

using namespace std;

class TrieImpl;

class Trie {
 public:
  Trie();
  ~Trie();

  // add adds a key. Note that keys must be added in order.
  void add(const string &key);
  // build builds indexes for lookup.
  void build();

  // lookup searches for query and returns its ID on success or -1 on failure.
  int64_t lookup(const string &query) const;

  // n_keys returns the number of keys.
  uint64_t n_keys() const;
  // n_nodes returns the number of nodes.
  uint64_t n_nodes() const;
  // size returns the approximate size of assigned memory.
  uint64_t size() const;

 private:
  TrieImpl *impl_;
};

}  // namespace louds

#endif  // LOUDS_TRIE_HPP
