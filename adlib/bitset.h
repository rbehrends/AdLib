#pragma once

#include "lib.h"

static inline Int bitcount(Word32 word) {
  word = word - ((word >> 1) & 0x55555555);
  word = (word & 0x33333333) + ((word >> 2) & 0x33333333);
  return (((word + (word >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

#ifdef HAVE_64BIT

static inline Int bitcount(Word64 word) {
  return bitcount((Word32) word) + bitcount(word >> 32);
}

#endif


class BitSet;

typedef List<BitSet *> BitMatrix;

class BitSet : public GC {
private:
  Int _bits;
  Int _words;
  Word32 *_data;
  void init(Int n);
  void resize(Int n);
  void expand(Int n);
  Int index(Word n) {
    return n >> 5;
  }
  Word bit(Word n) {
    return 1 << (n & 31);
  }

public:
  class Each {
  private:
    BitSet *_set;
    Int _i;
    Word _off;
    Word32 _mask;
    void next() {
      _i++;
      _mask <<= 1;
      if (_mask == 0) {
        _mask = 1;
        _off++;
      }
    }
    void skip() {
      while (_i < _set->_bits && (_set->_data[_off] & _mask) == 0) {
        next();
      }
    }

  public:
    Each(BitSet *set) {
      _set = set;
      _i = 0;
      _mask = 1;
      _off = 0;
      skip();
    }
    operator bool() {
      return _i < _set->_bits;
    }
    void operator++() {
      next();
      skip();
    }
    void operator++(int dummy) {
      next();
      skip();
    }
    Int operator*() {
      return _i;
    }
  };

  BitSet(Int n) {
    init(n);
  }
  Int len() {
    return _bits;
  }
  void zero();
  BitSet(BitSet *set) {
    init(set->_bits);
    memcpy(_data, set->_data, _words * sizeof(Word32));
  }
  BitSet *clone() {
    return new BitSet(this);
  }
  void set(Int i) {
    require(i < _bits, "index out of range");
    _data[index(i)] |= bit(i);
  }
  void clear(Int i) {
    require(i < _bits, "index out of range");
    _data[index(i)] &= ~bit(i);
  }
  bool test(Int i) {
    require(i < _bits, "index out of range");
    return (_data[index(i)] & bit(i)) != 0;
  }
  Int count();
  BitSet *complement();
  BitSet *complement_in_place();
  BitSet *union_with(BitSet *that);
  BitSet *union_in_place(BitSet *that);
  BitSet *intersect_with(BitSet *that);
  BitSet *intersect_in_place(BitSet *that);
  BitSet *diff_with(BitSet *that);
  BitSet *diff_in_place(BitSet *that);
  friend BitMatrix *Transpose(BitMatrix *mat);
  friend BitMatrix *TransitiveClosure(BitMatrix *mat);
};

template <Word _bits>
class FixedBitSet : public PtrFreeGC {
private:
  static const Word _words = (_bits + sizeof(Word32) * 8 - 1) / sizeof(Word32);
  Word32 _data[_words];
  void init() {
    memset(_data, 0, sizeof(_data));
  }
  void resize(Int n);
  void expand(Int n);
  Int index(Word n) {
    return n >> 5;
  }
  Word bit(Word n) {
    return 1 << (n & 31);
  }

public:
  class Each {
  private:
    FixedBitSet *_set;
    Int _i;
    Word _off;
    Word32 _mask;
    void next() {
      _i++;
      _mask <<= 1;
      if (_mask == 0) {
        _mask = 1;
        _off++;
      }
    }
    void skip() {
      while (_i < _bits && (_set->_data[_off] & _mask) == 0) {
        next();
      }
    }

  public:
    Each(BitSet *set) {
      _set = set;
      _i = 0;
      _mask = 1;
      _off = 0;
      skip();
    }
    operator bool() {
      return _i < _set->_bits;
    }
    void operator++() {
      next();
      skip();
    }
    void operator++(int dummy) {
      next();
      skip();
    }
    Int operator*() {
      return _i;
    }
  };

  FixedBitSet() {
    init();
  }
  Int len() {
    return _bits;
  }
  void zero() {
    init();
  }
  FixedBitSet(FixedBitSet<_bits> &set) {
    memcpy(_data, set._data, sizeof(_data));
  }
  BitSet *clone() {
    return new BitSet(this);
  }
  void set(Int i) {
    require(i < _bits, "index out of range");
    _data[index(i)] |= bit(i);
  }
  void clear(Int i) {
    require(i < _bits, "index out of range");
    _data[index(i)] &= ~bit(i);
  }
  bool test(Int i) {
    require(i < _bits, "index out of range");
    return (_data[index(i)] & bit(i)) != 0;
  }
  Int count() {
    Int result = 0;
    for (Int i = 0; i < _words; i++) {
      result += bitcount(_data[i]);
    }
    return result;
  }
  FixedBitSet& complement() {
    for (Int i = 0; i < _words; i++) {
      _data[i] = ~_data[i];
    }
    const Word n = _bits & 31;
    if (n != 0) {
      // zero top bits
      _data[_words - 1] &= ((1U << n) - 1);
    }
    return *this;
  }
  FixedBitSet<_bits>& union_with(FixedBitSet<_bits> &that) {
    for (Int i = 0; i <_words; i++)
      _data[i] |= that._data[i];
    return *this;
  }
  FixedBitSet<_bits>& intersect_with(FixedBitSet<_bits> &that) {
    for (Int i = 0; i <_words; i++)
      _data[i] &= that._data[i];
    return *this;
  }
  FixedBitSet& diff_with(FixedBitSet &that) {
    for (Int i = 0; i <_words; i++)
      _data[i] &= ~that._data[i];
    return *this;
  }
};

BitMatrix *MakeBitMatrix(Int n, Int m);
bool IsMatrix(BitMatrix *mat);
BitMatrix *Clone(BitMatrix *mat);
BitMatrix *Transpose(BitMatrix *mat);
BitMatrix *TransitiveClosure(BitMatrix *mat);
