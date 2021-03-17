#pragma once

#include "lib.h"

// Note: this is not an enum. A low bit of 0 indicates a value of either
// SLOT_EMPTY or SLOT_DELETED. A low bit of 1 (== SLOT_OCCUPIED)
// indicates that the top bits contain an abbreviated hash value.

#define SLOT_EMPTY 0
#define SLOT_OCCUPIED 1
#define SLOT_DELETED 2

#define MAP_ARGS <typename K, typename V, int Cmp(K, K), Word Hash(K)>
#define MAP_T Map<K, V, Cmp, Hash>

template <typename K, typename V>
struct Assoc : public GC {
  K key;
  V value;
};

template <typename K, typename V, int Cmp(K, K) = Cmp, Word Hash(K) = Hash>
class Map : public GC {
  template <typename KF, typename VF, int CmpF(KF, KF), Word HashF(KF)>
  friend class Map;
private:
  static const Int _minsize = 8;
  Int _count;
  Int _size;
  Int _deleted;
  K *_keys;
  V *_values;
  // The _state array contains either SLOT_EMPTY, SLOT_DELETED or
  // an abbreviated hash value with the lowest bit (SLOT_OCCUPIED)
  // set. Rather than storing the entire hash value, we only store
  // 7 bits of it, expecting this to still eliminate 99% of calls
  // to the compare function.
  Byte *_state;
  void resize(Int newsize) {
    _keys = (K *) GC_MALLOC(newsize * sizeof(K));
    _values = (V *) GC_MALLOC(newsize * sizeof(V));
    _state = (Byte *) GC_MALLOC_ATOMIC(newsize);
    memset(_state, SLOT_EMPTY, newsize);
    _size = newsize;
  }
  void uncheckedAdd(K key, V value, bool replace = false);
  void rebuild();
  Word next(Word pos, Word hash) {
    return (pos - hash) * 5 + 1 + hash;
  }

public:
  class Each : public GC {
  private:
    const Map *_map;
    Int _i;
    void skip() {
      while (_i < _map->_size && (_map->_state[_i] & SLOT_OCCUPIED) == 0)
        _i++;
    }

  public:
    Each(const Map *map, Int i = 0) : _map(map), _i(i) {
      skip();
    }
    operator bool() {
      return _i < _map->_size;
    }
    void operator++() {
      _i++;
      skip();
    }
    void operator++(int dummy) {
      _i++;
      skip();
    }
    K &key() {
      return _map->_keys[_i];
    }
    V &value() {
      return _map->_values[_i];
    }
    Assoc<K, V> pair() {
      Assoc<K, V> assoc;
      assoc.key = _map->_keys[_i];
      assoc.value = _map->_values[_i];
      return assoc;
    }
  };
  class EachPair : public Each {
  public:
    EachPair(const Map *map, Int i = 0) : Each(map, i) { }
    Assoc<K, V> operator*() {
      return Assoc<K,V>(Each::key(), Each::value());
    }
  };
  class EachKey : public Each {
  public:
    EachKey(const Map *map, Int i = 0) : Each(map, i) { }
    K& operator*() {
      return Each::key();
    }
  };
  class EachValue : public Each {
  public:
    EachValue(const Map *map, Int i = 0) : Each(map, i) { }
    V& operator*() {
      return Each::value();
    }
  };
  class Keys : public GC {
    const Map* _map;
  public:
    Keys(const Map* map) : _map(map) { }
    EachKey begin() {
      return EachKey(_map, 0);
    }
    EachKey end() {
      return EachKey(_map, _map->_size);
    }
  };
  class Values : public GC {
    const Map* _map;
  public:
    Values(const Map* map) : _map(map) { }
    EachValue begin() {
      return EachValue(_map, 0);
    }
    EachValue end() {
      return EachValue(_map, _map->_size);
    }
  };
  class Items : public GC {
    const Map* _map;
  public:
    Items(const Map* map) : _map(map) { }
    EachPair begin() {
      return EachPair(_map, 0);
    }
    EachPair end() {
      return EachPair(_map, _map->_size);
    }
  };
  Map();
  Map(MAP_T *map, bool copy = true);
  Map(List<K> *keys, List<V> *values);
  Map(List<K> *keys, List<V> *values, CmpFunc(K, cmp), HashFunc(K, hash));
  MAP_T *clone() {
    return new Map(this);
  }
  Int count() {
    return _count;
  }
  MAP_T *add(K key, V value, bool replace = false);
  MAP_T *add(Assoc<K, V> assoc, bool replace = false) {
    add(assoc.key, assoc.value, replace);
    return this;
  }
  MAP_T *add(List<Assoc<K, V> > *arr, bool replace = false);
  MAP_T *add(List<K> *keys, List<V> *values, bool replace = false);
  bool remove(K key);
  bool contains(K key);
  bool find(K key, V &value);
  V get(K key, V if_absent);
  Opt<V> get(K key);
  V at(K key);
  List<K> *keys();
  List<V> *values();
  List<Assoc<K, V> > *pairs();
  bool eq(MAP_T *that);
  void clear() {
    resize(_minsize);
    _count = 0;
    _deleted = 0;
  }
  MAP_T *union_with(MAP_T *that, bool replace = false);
  MAP_T *union_in_place(MAP_T *that, bool replace = false);
  MAP_T *intersect_with(MAP_T *that);
  MAP_T *intersect_in_place(MAP_T *that);
  MAP_T *diff_with(MAP_T *that);
  MAP_T *diff_in_place(MAP_T *that);
  template <typename A, typename F>
  A fold(A init, F foldfunc);
  template <typename M, typename F>
  M *map_values(F mapfunc);
  template <typename F>
  MAP_T *map_values(F mapfunc) {
    return map_values<MAP_T, F>(mapfunc);
  }
  template <typename M, typename F, typename F2>
  M *map_pairs(F mapfunc, F2 mapfunc2);
  template <typename F, typename F2>
  MAP_T *map_pairs(F mapfunc, F2 mapfunc2) {
    return map_pairs<MAP_T, F, F2>(mapfunc, mapfunc2);
  }
  template <typename F>
  MAP_T *filter(F filterfunc);
  template <typename F>
  void iter(F iterfunc);
  Items each() {
    return Items(this);
  }
  Items each_pair() {
    return Items(this);
  }
  Keys each_key() {
    return Keys(this);
  }
  Values each_value() {
    return Values(this);
  }
};

template MAP_ARGS
void MAP_T::rebuild() {
  Int size = _size;
  Int newsize = nextPow2(_count * 2);
  if (newsize < _minsize)
    newsize = _minsize;
  K *keys = _keys;
  V *values = _values;
  Byte *state = _state;
  _count = 0;
  _deleted = 0;
  resize(newsize);
  for (Int i = 0; i < size; i++) {
    if (state[i] & SLOT_OCCUPIED)
      uncheckedAdd(keys[i], values[i]);
  }
}

template MAP_ARGS
MAP_T::Map() {
  resize(_minsize);
  _count = 0;
  _deleted = 0;
}

template MAP_ARGS
MAP_T::Map(List<K> *keys, List<V> *values) {
  resize(_minsize);
  _count = 0;
  _deleted = 0;
  add(keys, values);
}

template MAP_ARGS
MAP_T::Map(
    List<K> *keys, List<V> *values, CmpFunc(K, cmp), HashFunc(K, hash)) {
  resize(_minsize);
  _count = 0;
  _deleted = 0;
  add(keys, values);
}

template MAP_ARGS
MAP_T::Map(MAP_T *map, bool copy) {
  if (copy) {
    resize(map->_size);
    memcpy(_keys, map->_keys, sizeof(K) * _size);
    memcpy(_values, map->_values, sizeof(V) * _size);
    memcpy(_state, map->_state, _size);
    _count = map->_count;
    _deleted = map->_deleted;
  } else {
    resize(_minsize);
    _count = 0;
    _deleted = 0;
  }
}

#define INIT_HASH_LOOP(key) \
  Word mask = _size - 1; \
  Word hash = Hash(key); \
  Word pos = hash & mask; \
  Byte occ = FibHash(hash, 8) | SLOT_OCCUPIED

template MAP_ARGS
void MAP_T::uncheckedAdd(K key, V value, bool replace) {
  INIT_HASH_LOOP(key);
  Int freepos = -1;
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_keys[pos], key) == 0) {
      if (replace) {
        _keys[pos] = key;
        _values[pos] = value;
      }
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
  _keys[pos] = key;
  _values[pos] = value;
  _state[pos] = occ;
  _count++;
}

template MAP_ARGS
MAP_T *MAP_T::add(K key, V value, bool replace) {
  if ((_count + _deleted) * 3 / 2 >= _size)
    rebuild();
  uncheckedAdd(key, value, replace);
  return this;
}

template MAP_ARGS
MAP_T *MAP_T::add(List<Assoc<K, V> > *arr, bool replace) {
  for (Int i = 0; i < arr->len(); i++)
    add(arr->at(i));
  return this;
}

template MAP_ARGS
MAP_T *MAP_T::add(List<K> *keys, List<V> *values, bool replace) {
  require(keys->len() == values->len(), "mismatched array sizes");
  for (Int i = 0; i < keys->len(); i++) {
    add(keys->at(i), values->at(i));
  }
  return this;
}

template MAP_ARGS
bool MAP_T::remove(K key) {
  INIT_HASH_LOOP(key);
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_keys[pos], key) == 0) {
      memset(_keys + pos, 0, sizeof(K));
      memset(_values + pos, 0, sizeof(V));
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

template MAP_ARGS
bool MAP_T::find(K key, V &value) {
  INIT_HASH_LOOP(key);
  while (_state[pos] != SLOT_EMPTY) {
    if (_state[pos] == occ && Cmp(_keys[pos], key) == 0) {
      value = _values[pos];
      return true;
    }
    pos = next(pos, hash) & mask;
  }
  return false;
}

template MAP_ARGS
V MAP_T::get(K key, V if_absent) {
  V result;
  if (find(key, result))
    return result;
  else
    return if_absent;
}

template MAP_ARGS
Opt<V> MAP_T::get(K key) {
  V result;
  if (find(key, result))
    return Some<V>(result);
  else
    return None<V>();
}

template MAP_ARGS
V MAP_T::at(K key) {
  V result;
  require(find(key, result), "key not found");
  return result;
}

template MAP_ARGS
bool MAP_T::contains(K key) {
  V value;
  return find(key, value);
}

template MAP_ARGS
List<K> *MAP_T::keys() {
  List<K> *result = new List<K>(_count);
  for (Int i = 0; i < _size; i++) {
    if (_state[i] & SLOT_OCCUPIED)
      result->add(_keys[i]);
  }
  return result;
}

template MAP_ARGS
List<V> *MAP_T::values() {
  List<V> *result = new List<V>(_count);
  for (Int i = 0; i < _size; i++) {
    if (_state[i] & SLOT_OCCUPIED)
      result->add(_values[i]);
  }
  return result;
}

template MAP_ARGS
List<Assoc<K, V> > *MAP_T::pairs() {
  List<Assoc<K, V> > *result = new List<Assoc<K, V> >(_count);
  for (Int i = 0; i < _size; i++) {
    if (_state[i] & SLOT_OCCUPIED) {
      Assoc<K, V> m;
      m.key = _keys[i];
      m.value = _values[i];
      result->add(m);
    }
  }
  return result;
}

template MAP_ARGS
MAP_T *MAP_T::union_in_place(MAP_T *that, bool replace) {
  if (this == that)
    return this;
  for (Each it(that); it; it++) {
    add(it.key(), it.value(), replace);
  }
  return this;
}

template MAP_ARGS
MAP_T *MAP_T::union_with(MAP_T *that, bool replace) {
  return clone()->union_in_place(that, replace);
}

template MAP_ARGS
MAP_T *MAP_T::diff_in_place(MAP_T *that) {
  if (this == that) {
    clear();
    return this;
  }
  for (Each it(that); it; it++) {
    remove(it.key());
  }
  return this;
}

template MAP_ARGS
MAP_T *MAP_T::diff_with(MAP_T *that) {
  return clone()->diff_in_place(that);
}

template MAP_ARGS
MAP_T *MAP_T::intersect_with(MAP_T *that) {
  MAP_T *result = new MAP_T();
  for (Each it(this); it; it++) {
    if (that->contains(it.key()))
      result->add(it.key(), it.value());
  }
  return result;
}

template MAP_ARGS
MAP_T *MAP_T::intersect_in_place(MAP_T *that) {
  if (this == that)
    return this;
  MAP_T *tmp = intersect_with(that);
  *this = *tmp;
  return this;
}

template MAP_ARGS
bool MAP_T::eq(MAP_T *that) {
  // FIXME: compare values
  if (_count != that->_count)
    return false;
  for (Each it(this); it; it++) {
    if (!that->contains(it.key()))
      return false;
  }
  return true;
}

template MAP_ARGS
template <typename A, typename F>
A MAP_T::fold(A init, F foldfunc) {
  A result = init;
  for (Each it(this); it; it++) {
    result = foldfunc(result, it.key(), it.value());
  }
  return result;
}

template MAP_ARGS
template <typename M, typename F>
M *MAP_T::map_values(F mapfunc) {
  M *result = new M();
  for (Each it(this); it; it++) {
    result->add(it.key(), mapfunc(it.value()));
  }
  return result;
}

template MAP_ARGS
template <typename M, typename F, typename F2>
M *MAP_T::map_pairs(F mapfunc, F2 mapfunc2) {
  M *result = new M();
  for (Each it(this); it; it++) {
    result->add(mapfunc(it.key()), mapfunc2(it.value()));
  }
  return result;
}

template MAP_ARGS
template <typename F>
MAP_T *MAP_T::filter(F filterfunc) {
  MAP_T *result = new MAP_T();
  for (Each it(this); it; it++) {
    if (filterfunc(it.key(), it.value()))
      result->add(it.key(), it.value());
  }
  return result;
}

template MAP_ARGS
template <typename F>
void MAP_T::iter(F iterfunc) {
  for (Each it(this); it; it++) {
    iterfunc(*it);
  }
}

typedef Map<Str *, Str *> Dict;

#undef SLOT_EMPTY
#undef SLOT_OCCUPIED
#undef SLOT_DELETED
#undef MAP_ARGS
#undef MAP_T

#undef INIT_HASH_LOOP
