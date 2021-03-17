#pragma once

#include "lib.h"

// Note: this is not an enum. A low bit of 0 indicates a value of either
// SLOT_EMPTY or SLOT_DELETED. A low bit of 1 (== SLOT_OCCUPIED)
// indicates that the top bits contain an abbreviated hash value.

#define SLOT_EMPTY 0
#define SLOT_OCCUPIED 1
#define SLOT_DELETED 2
#define SET_ARGS <typename T, int Cmp(T, T), Word Hash(T)>
#define SET_T Set<T, Cmp, Hash>

template <typename T, int Cmp(T, T) = Cmp, Word Hash(T) = Hash>
class Set : public GC {
  template <typename TF, int CmpF(TF, TF), Word HashF(TF)>
  friend class Set;
private:
  static const Int _minsize = 8;
  Int _count;
  Int _size;
  Int _deleted;
  T *_data;
  // The _state array contains either SLOT_EMPTY, SLOT_DELETED or
  // an abbreviated hash value with the lowest bit (SLOT_OCCUPIED)
  // set. Rather than storing the entire hash value, we only store
  // 7 bits of it, expecting this to still eliminate 99% of calls
  // to the compare function.
  Byte *_state;
  void resize(Int newsize) {
    _data = (T *) GC_MALLOC(newsize * sizeof(T));
    _state = (Byte *) GC_MALLOC_ATOMIC(newsize);
    memset(_state, SLOT_EMPTY, newsize);
    _size = newsize;
  }
  void uncheckedAdd(T item, bool replace = false);
  void rebuild();
  Word next(Word pos, Word hash) {
    return (pos - hash) * 5 + 1 + hash;
  }

public:
  class Each {
  private:
    const Set *_set;
    Int _i;
    void skip() {
      while (_i < _set->_size && (_set->_state[_i] & SLOT_OCCUPIED) == 0)
        _i++;
    }

  public:
    Each(const Set *set, Int i = 0) : _set(set), _i(i) {
      skip();
    }
    operator bool() {
      return _i < _set->_size;
    }
    void operator++() {
      _i++;
      skip();
    }
    void operator++(int dummy) {
      _i++;
      skip();
    }
    T &operator*() const {
      return _set->_data[_i];
    }
    T operator->() const {
      return _set->_data[_i];
    }
  };
  class Items {
  private:
    const Set *_set;
  public:
    Items(const Set *set) : _set(set) { }
    Each begin() const {
      return Each(_set, 0);
    }
    Each end() const {
      return Each(_set, _set->_size);
    }
  };
  Set();
  Set(const SET_T *set);
  Set(List<T> *);
  SET_T *clone() const {
    return new Set(this);
  }
  Int count() const {
    return _count;
  }
  SET_T *add(const T item, bool replace = false);
  SET_T *add(const List<T> *item, bool replace = false);
  SET_T &operator<<(const T item) {
    return *add(item);
  }
  bool remove(const T item);
  bool contains(const T item) const;
  bool at(const T item) const {
    return contains(item);
  }
  T *find(const T item) const;
  T get_or_add(T item);
  List<T> *items() const;
  bool eq(const SET_T *that) const;
  void clear() {
    resize(_minsize);
    _count = 0;
    _deleted = 0;
  }
  SET_T *union_with(const SET_T *that, bool replace = false) const;
  SET_T *union_in_place(const SET_T *that, bool replace = false);
  SET_T *intersect_with(const SET_T *that) const;
  SET_T *intersect_in_place(const SET_T *that);
  SET_T *diff_with(const SET_T *that) const;
  SET_T *diff_in_place(const SET_T *that);
  template <typename A, typename F>
  A fold(const A init, F foldfunc) const;
  template <typename S, typename F>
  S *map(F mapfunc) const;
  template <typename F>
  SET_T *map(F mapfunc) const {
    return map<SET_T, F>(mapfunc);
  }
  template <typename F>
  SET_T *filter(F filterfunc) const;
  template <typename F>
  void iter(F iterfunc);
  Items each() const {
    return Items(this);
  }
};

template SET_ARGS
void SET_T::rebuild() {
  Int size = _size;
  Int newsize = nextPow2(_count * 2);
  if (newsize < _minsize)
    newsize = _minsize;
  T *data = _data;
  Byte *state = _state;
  _count = 0;
  _deleted = 0;
  resize(newsize);
  for (Int i = 0; i < size; i++) {
    if ((state[i] & SLOT_OCCUPIED) != 0)
      uncheckedAdd(data[i]);
  }
}

template SET_ARGS
SET_T::Set() {
  resize(_minsize);
  _count = 0;
  _deleted = 0;
}

template SET_ARGS
SET_T::Set(const SET_T *set) {
    resize(set->_size);
    memcpy(_data, set->_data, sizeof(T) * _size);
    memcpy(_state, set->_state, _size);
    _count = set->_count;
    _deleted = set->_deleted;
}

template SET_ARGS
SET_T::Set(List<T> *arr) {
  resize(_minsize);
  _count = 0;
  _deleted = 0;
  add(arr);
}

#define INIT_HASH_LOOP(item) \
  Word mask = _size - 1; \
  Word hash = Hash(item); \
  Word pos = hash & mask; \
  Byte occ = FibHash(hash, 8) | SLOT_OCCUPIED

template SET_ARGS
void SET_T::uncheckedAdd(T item, bool replace) {
  INIT_HASH_LOOP(item);
  Int freepos = -1;
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_data[pos], item) == 0) {
      if (replace)
        _data[pos] = item;
      return;
    }
    if (_state[pos] == SLOT_DELETED && freepos < 0)
      freepos = pos;
    pos = next(pos, hash) & mask;
  }
  if (freepos >= 0) {
    _deleted--;
    pos = freepos;
  }
  _data[pos] = item;
  _state[pos] = occ;
  _count++;
}

template SET_ARGS
T SET_T::get_or_add(T item) {
  if ((_count + _deleted) * 3 / 2 >= _size)
    rebuild();
  INIT_HASH_LOOP(item);
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_data[pos], item) == 0) {
      return _data[pos];
    }
    pos = next(pos, hash) & mask;
  }
  if (_state[pos] == SLOT_DELETED) {
    _deleted--;
  }
  _data[pos] = item;
  _state[pos] = occ;
  _count++;
  return item;
}

template SET_ARGS
SET_T *SET_T::add(T item, bool replace) {
  if ((_count + _deleted) * 3 / 2 >= _size)
    rebuild();
  uncheckedAdd(item, replace);
  return this;
}

template SET_ARGS
SET_T *SET_T::add(const List<T> *arr, bool replace) {
  if ((_count + _deleted) * 3 / 2 >= _size)
    rebuild();
  for (Int i = 0; i < arr->len(); i++)
    add(arr->at(i), replace);
  return this;
}

template SET_ARGS
bool SET_T::remove(const T item) {
  INIT_HASH_LOOP(item);
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_data[pos], item) == 0) {
      memset(_data + pos, 0, sizeof(T));
      _state[pos] = SLOT_DELETED;
      _count--;
      _deleted++;
      if ((_count + _deleted) * 3 / 2 > _size || _deleted >= _count)
        rebuild();
      return 1;
    }
    pos = next(pos, hash) & mask;
  }
  return 0;
}

template SET_ARGS
T *SET_T::find(const T item) const {
  INIT_HASH_LOOP(item);
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_data[pos], item) == 0)
      return _data + pos;
    pos = next(pos, hash) & mask;
  }
  return NULL;
}

template SET_ARGS
bool SET_T::contains(const T item) const {
  return find(item) != NULL;
}

template SET_ARGS
List<T> *SET_T::items() const {
  List<T> *result = new List<T>(_count);
  for (Int i = 0; i < _size; i++) {
    if (_state[i] & SLOT_OCCUPIED)
      result->add(_data[i]);
  }
  return result;
}

template SET_ARGS
SET_T *SET_T::union_in_place(const SET_T *that, bool replace) {
  if (this == that)
    return this;
  for (Each it(that); it; it++) {
    add(*it, replace);
  }
  return this;
}

template SET_ARGS
SET_T *SET_T::union_with(const SET_T *that, bool replace) const {
  return clone()->union_in_place(that, replace);
}

template SET_ARGS
SET_T *SET_T::diff_in_place(const SET_T *that) {
  if (this == that) {
    clear();
    return this;
  }
  for (Each it(that); it; it++) {
    remove(*it);
  }
  return this;
}

template SET_ARGS
SET_T *SET_T::diff_with(const SET_T *that) const {
  return clone()->diff_in_place(that);
}

template SET_ARGS
SET_T *SET_T::intersect_with(const SET_T *that) const {
  SET_T *result = new SET_T();
  for (Each it(this); it; it++) {
    if (that->contains(*it))
      result->add(*it);
  }
  return result;
}

template SET_ARGS
SET_T *SET_T::intersect_in_place(const SET_T *that) {
  if (this == that)
    return this;
  SET_T *tmp = intersect_with(that);
  *this = *tmp;
  return this;
}

template SET_ARGS
bool SET_T::eq(const SET_T *that) const {
  if (_count != that->_count)
    return false;
  for (Each it(this); it; it++) {
    if (!that->contains(*it))
      return false;
  }
  return true;
}

template SET_ARGS
template <typename A, typename F>
A SET_T::fold(const A init, F foldfunc) const {
  A result = init;
  for (Each it(this); it; it++) {
    result = foldfunc(result, *it);
  }
  return result;
}

template SET_ARGS
template <typename S, typename F>
S *SET_T::map(F mapfunc) const {
  S *result = new S();
  for (Each it(this); it; it++) {
    result->add(mapfunc(*it));
  }
  return result;
}


template SET_ARGS
template <typename F>
SET_T *SET_T::filter(F filterfunc) const {
  SET_T *result = new SET_T();
  for (Each it(this); it; it++) {
    if (filterfunc(*it))
      result->add(*it);
  }
  return result;
}

template SET_ARGS
template <typename F>
void SET_T::iter(F iterfunc) {
  for (Each it(this); it; it++) {
    iterfunc(*it);
  }
}

typedef Set<Str *> StrSet;

static inline Str *S(const StrSet *set, const char *sep = " ") {
  return StrJoin(set->items()->sort(StrCmp), sep);
}

#undef SLOT_EMPTY
#undef SLOT_OCCUPIED
#undef SLOT_DELETED
#undef SET_ARGS
#undef SET_T

#undef INIT_HASH_LOOP
