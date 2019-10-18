#include "louds-trie.hpp"

#include <x86intrin.h>

#include <cassert>
#include <vector>

namespace louds {

using namespace std;

namespace {

class BitVector {
 public:
  BitVector() : words_(), ranks_(), selects_(), n_bits_(0) {}
  ~BitVector() {}

  uint64_t get(uint64_t i) const {
    return (words_[i / 64] >> (i % 64)) & 1UL;
  }
  void set(uint64_t i, uint64_t bit) {
    if (bit) {
      words_[i / 64] |= (1UL << (i % 64));
    } else {
      words_[i / 64] &= ~(1UL << (i % 64));
    }
  }

  void add(uint64_t bit) {
    if (n_bits_ % 256 == 0) {
      words_.resize((n_bits_ + 256) / 64);
    }
    set(n_bits_, bit);
    ++n_bits_;
  }

  // build builds indexes for select.
  void build();

  // select returns the position of the (i+1)-th 0-bit.
  uint64_t select(uint64_t i) const;

  uint64_t n_bits() const {
    return n_bits_;
  }
  uint64_t size() const {
    return sizeof(uint64_t) * words_.size()
      + sizeof(Rank) * ranks_.size()
      + sizeof(uint32_t) * selects_.size();
  }

 private:
  vector<uint64_t> words_;
  struct Rank {
    uint32_t abs_hi;
    uint8_t abs_lo;
    uint8_t rels[3];

    uint64_t abs() const {
      return ((uint64_t)abs_hi << 8) | abs_lo;
    }
    void set_abs(uint64_t abs) {
      abs_hi = (uint32_t)(abs >> 8);
      abs_lo = (uint8_t)abs;
    }
  };
  vector<Rank> ranks_;
  vector<uint32_t> selects_;
  uint64_t n_bits_;
};

void BitVector::build() {
  uint64_t n_blocks = words_.size() / 4;
  uint64_t n_zeros = 0;
  ranks_.resize(n_blocks + 1);
  for (uint64_t block_id = 0; block_id < n_blocks; ++block_id) {
    ranks_[block_id].set_abs(n_zeros);
    for (uint64_t j = 0; j < 4; ++j) {
      if (j != 0) {
        uint64_t rel = n_zeros - ranks_[block_id].abs();
        ranks_[block_id].rels[j - 1] = rel;
      }

      uint64_t word_id = (block_id * 4) + j;
      uint64_t word = ~words_[word_id];
      uint64_t n_pops = __builtin_popcountll(word);
      uint64_t new_n_zeros = n_zeros + n_pops;
      if (((n_zeros + 255) / 256) != ((new_n_zeros + 255) / 256)) {
        uint64_t count = n_zeros;
        while (word != 0) {
          uint64_t pos = __builtin_ctzll(word);
          if (count % 256 == 0) {
            selects_.push_back(((word_id * 64) + pos) / 256);
            break;
          }
          word ^= 1UL << pos;
          ++count;
        }
      }
      n_zeros = new_n_zeros;
    }
  }
  ranks_.back().set_abs(n_zeros);
  selects_.push_back(words_.size() * 64 / 256);
}

uint64_t BitVector::select(uint64_t i) const {
  const uint64_t block_id = i / 256;
  uint64_t begin = selects_[block_id];
  uint64_t end = selects_[block_id + 1] + 1UL;
  if (begin + 10 >= end) {
    while (i >= ranks_[begin + 1].abs()) {
      ++begin;
    }
  } else {
    while (begin + 1 < end) {
      const uint64_t middle = (begin + end) / 2;
      if (i < ranks_[middle].abs()) {
        end = middle;
      } else {
        begin = middle;
      }
    }
  }
  const uint64_t rank_id = begin;
  i -= ranks_[rank_id].abs();

  uint64_t word_id = rank_id * 4;
  if (i < ranks_[rank_id].rels[1]) {
    if (i >= ranks_[rank_id].rels[0]) {
      word_id += 1;
      i -= ranks_[rank_id].rels[0];
    }
  } else if (i < ranks_[rank_id].rels[2]) {
    word_id += 2;
    i -= ranks_[rank_id].rels[1];
  } else {
    word_id += 3;
    i -= ranks_[rank_id].rels[2];
  }
  return (word_id * 64) + __builtin_ctzll(
    _pdep_u64(1UL << i, ~words_[word_id]));
}

struct Level {
  BitVector louds;
  BitVector outs;
  vector<uint8_t> labels;
  uint64_t offset;

  Level() : louds(), outs(), labels(), offset(0) {}

  uint64_t size() const;
};

uint64_t Level::size() const {
  return louds.size() + outs.size() + labels.size();
}

}  // namespace

class TrieImpl {
 public:
  TrieImpl();
  ~TrieImpl() {}

  void add(const string &key);
  void build();
  int64_t lookup(const string &query) const;

  uint64_t n_keys() const {
    return n_keys_;
  }
  uint64_t n_nodes() const {
    return n_nodes_;
  }
  uint64_t size() const {
    return size_;
  }

 private:
  vector<Level> levels_;
  uint64_t n_keys_;
  uint64_t n_nodes_;
  uint64_t size_;
  string last_key_;
};

TrieImpl::TrieImpl()
  : levels_(2), n_keys_(0), n_nodes_(1), size_(0), last_key_() {
  levels_[0].louds.add(1);
  levels_[0].louds.add(0);
  levels_[1].louds.add(0);
  levels_[0].outs.add(0);
  levels_[0].labels.push_back(' ');
}

void TrieImpl::add(const string &key) {
  assert(key > last_key_);
  if (key.empty()) {
    levels_[0].outs.set(0, 1);
    ++levels_[1].offset;
    ++n_keys_;
    return;
  }
  if (key.length() + 1 >= levels_.size()) {
    levels_.resize(key.length() + 2);
  }
  uint64_t i = 0;
  for ( ; i < key.length(); ++i) {
    Level &level = levels_[i + 1];
    uint8_t byte = key[i];
    if ((i == last_key_.length()) || (byte != level.labels.back())) {
      level.louds.set(levels_[i + 1].louds.n_bits() - 1, 1);
      level.louds.add(0);
      level.outs.add(0);
      level.labels.push_back(key[i]);
      ++n_nodes_;
      break;
    }
  }
  for (++i; i < key.length(); ++i) {
    Level &level = levels_[i + 1];
    level.louds.add(1);
    level.louds.add(0);
    level.outs.add(0);
    level.labels.push_back(key[i]);
    ++n_nodes_;
  }
  levels_[key.length() + 1].louds.add(0);
  ++levels_[key.length() + 1].offset;
  levels_[key.length()].outs.set(levels_[key.length()].outs.n_bits() - 1, 1);
  ++n_keys_;
  last_key_ = key;
}

void TrieImpl::build() {
  uint64_t offset = 0;
  for (uint64_t i = 0; i < levels_.size(); ++i) {
    Level &level = levels_[i];
    level.louds.build();
    offset += levels_[i].offset;
    level.offset = offset;
    size_ += level.size();
  }
}

int64_t TrieImpl::lookup(const string &query) const {
  if (query.length() >= levels_.size()) {
    return false;
  }
  uint64_t node_id = 0;
  uint64_t rank = 0;
  for (uint64_t i = 0; i < query.length(); ++i) {
    const Level &level = levels_[i + 1];
    if (rank != 0) {
      node_id = level.louds.select(rank - 1) + 1;
      rank = node_id - rank;
    } else {
      node_id = 0;
    }
    for (uint8_t byte = query[i]; ; ++node_id, ++rank) {
      if (!level.louds.get(node_id) || level.labels[rank] > byte) {
        return -1;
      }
      if (level.labels[rank] == byte) {
        break;
      }
    }
  }
  const Level &level = levels_[query.length()];
  if (!level.outs.get(rank)) {
    return false;
  }
  return level.offset + rank;
}

Trie::Trie() : impl_(new TrieImpl) {}

Trie::~Trie() {
  delete impl_;
}

void Trie::add(const string &key) {
  return impl_->add(key);
}

void Trie::build() {
  impl_->build();
}

int64_t Trie::lookup(const string &query) const {
  return impl_->lookup(query);
}

uint64_t Trie::n_keys() const {
  return impl_->n_keys();
}

uint64_t Trie::n_nodes() const {
  return impl_->n_nodes();
}

uint64_t Trie::size() const {
  return impl_->size();
}

}  // namespace louds

