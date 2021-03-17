#pragma once

template <typename T>
class List : public GC {
  template <typename U>
  friend class List;
private:
  Int _len;
  Int _cap;
  T *_data;
  void resize(Int newcap) {
    T *_newdata = (T *) GC_MALLOC(newcap * sizeof(T));
    memcpy(_newdata, _data, _len * sizeof(T));
    _data = _newdata;
    _cap = newcap;
  }
  void init(Int cap) {
    _len = 0;
    _cap = cap;
    _data = (T *) GC_MALLOC(cap * sizeof(T));
  }
  void init(const T *p, Int cap) {
    _len = _cap = cap;
    _data = (T *) GC_MALLOC(cap * sizeof(T));
    memcpy(_data, p, _len * sizeof(T));
  }

public:
  class Each : public GC {
  private:
    const List *_arr;
    Int _i;

  public:
    Each(const List *arr, Int i = 0): _arr(arr), _i(i) { }
    Int index() const {
      return _i;
    }
    operator bool() const {
      return _i < _arr->_len;
    }
    void operator++() {
      _i++;
    }
    void operator++(int dummy) {
      _i++;
    }
    bool operator==(Each &other) const {
      return _i == other._i && _arr == other._arr;
    }
    bool operator!=(Each &other) {
      return _i != other._i || _arr != other._arr;
    }
    T &operator*() const {
      return _arr->_data[_i];
    }
    T operator->() const {
      return _arr->_data[_i];
    }
  };
  class Items : public GC {
    const List *_list;
  public:
    Items(const List *list) : _list(list) { }
    Each begin() const {
      return Each(_list, 0);
    }
    Each end() const {
      return Each(_list, _list->_len);
    }
  };
  class Range : public GC {
    const List *_list;
    const Int _from, _to;
  public:
    Range(const List *list, Int from, Int to)
        : _list(list)
        , _from(from)
        , _to(to) {
      require(from >= 0 && from <= list->_len, "invalid start of range");
      require(to >= 0 && to <= list-> _len, "invalid end of range");
    }
    Each begin() {
      return Each(_list, _from);
    }
    Each end() {
      return Each(_list, _to);
    }
  };
  Items each() const {
    return Items(this);
  }
  Range each(Int from) const {
    return Range(this, from, _len);
  }
  Range each(Int from, Int to) const {
    return Range(this, from, to);
  }
  List(Int n, const T *data) {
    init(data, n);
  }
  List(const T *data, T sentinel) {
    Int n = 0;
    while (data[n] != sentinel)
      n++;
    init(data, n);
  }
  template <typename D, typename F>
  List(const D *data, D sentinel, F initfunc);
  template <typename D, typename F>
  List(Int n, const D *data, F initfunc);
  List(Int size) {
    init(size);
  }
  List(Int size, T arg1) {
    init(size);
    add(arg1);
  }
  List(Int size, T arg1, T arg2) {
    init(size);
    add(arg1);
    add(arg2);
  }
  List(Int size, T arg1, T arg2, T arg3) {
    init(size);
    add(arg1);
    add(arg2);
    add(arg3);
  }
  List(Int size, T arg1, T arg2, T arg3, T arg4) {
    init(size);
    add(arg1);
    add(arg2);
    add(arg3);
    add(arg4);
  }
  List() {
    init(2);
  }
  List(const List *arr) {
    init(arr->_data, arr->_len);
  }
  List *clone() const {
    return new List<T>(this);
  }
  List<T> *expand(Int newlen) {
    if (newlen > _cap)
      resize(nextPow2(newlen));
    return this;
  }
  List<T> *shrink(bool fit = true) {
    if (_len != _cap) {
      if (fit || _len * 3 / 2 + 1 < _cap)
        resize(_len);
    }
    return this;
  }
  List<T> *add(const T item) {
    expand(_len + 1);
    _data[_len++] = item;
    return this;
  }
  List<T> *add(const T *p, Int n) {
    expand(_len + n);
    memcpy(_data + _len, p, n * sizeof(T));
    _len += n;
    return this;
  }
  List<T> *add(const List<T> *other) {
    return add(other->_data, other->_len);
  }
  List<T> *pop(Int n = 1) {
    if (n > _len)
      n = _len;
    memset(_data + _len - n, 0, n * sizeof(T));
    _len -= n;
    return this;
  }
  List<T> *remove(Int at);
  List<T> *remove(Int start, Int count);
  List<T> *set_len(Int len);
  List<T> *fill(Int start, Int count, T value);
  List<T> *subarr(Int start, Int count) const;
  List<T> *range_incl(Int start, Int end) const {
    return subarr(start, end - start + 1);
  }
  List<T> *range_excl(Int start, Int end) const {
    return subarr(start, end - start);
  }
  T shift();
  T first() const {
    require(_len > 0, "last element of empty array");
    return _data[0];
  }
  T last() const {
    require(_len > 0, "last element of empty array");
    return _data[_len - 1];
  }
  Int len() const {
    return _len;
  }
  Int count() const {
    return _len;
  }
  bool eq(List<T> *that) const;
  T &item(Int i) {
    require(0 <= i && i < _len, "index out of range");
    return _data[i];
  }
  T &at(Int i) const {
    require(0 <= i && i < _len, "index out of range");
    return _data[i];
  }
  T *c_mem() const {
    return _data;
  }
  template <typename U, typename F>
  List<U> *map(F mapfunc) const;
  template <typename F>
  List<T> *map_in_place(F mapfunc);
  template <typename U, typename F>
  List<U> *map_indexed(F mapfunc) const;
  template <typename F>
  List<T> *map_indexed_in_place(F mapfunc);
  template <typename F>
  List<T> *filter(F filterfunc) const;
  template <typename F>
  List<T> *filter_in_place(F filterfunc);
  template <typename A, typename F>
  A fold(A init, F foldfunc) const;
  template <typename F>
  List<T> *sort(F cmpfunc) const;
  template <typename F>
  List<T> *sort_in_place(F cmpfunc);
  template <typename F>
  void iter(F iterfunc);
  template <typename F>
  void iter_indexed(F iterfunc);
};

template <typename T>
template <typename D, typename F>
List<T>::List(Int n, const D *data, F initfunc) {
  init(n);
  for (Int i = 0; i < n; i++) {
    add(initfunc(data[i]));
  }
}
template <typename T>
template <typename D, typename F>
List<T>::List(const D *data, D sentinel, F initfunc) {
  Int n = 0;
  while (data[n] != sentinel)
    n++;
  init(n);
  for (Int i = 0; i < n; i++) {
    add(initfunc(data[i]));
  }
}

template <typename T>
template <typename F>
List<T> *List<T>::map_indexed_in_place(F mapfunc) {
  for (Int i = 0; i < _len; i++) {
    _data[i] = mapfunc(i, _data[i]);
  }
  return this;
}

template <typename T>
template <typename U, typename F>
List<U> *List<T>::map_indexed(F mapfunc) const {
  List<U> *result = new List<U>(_cap);
  result->_len = _len;
  for (Int i = 0; i < _len; i++) {
    result->_data[i] = mapfunc(i, _data[i]);
  }
  return result;
}

template <typename T>
template <typename F>
List<T> *List<T>::map_in_place(F mapfunc) {
  for (Int i = 0; i < _len; i++) {
    _data[i] = mapfunc(_data[i]);
  }
  return this;
}

template <typename T>
template <typename U, typename F>
List<U> *List<T>::map(F mapfunc) const {
  List<U> *result = new List<U>(_cap);
  result->_len = _len;
  for (Int i = 0; i < _len; i++) {
    result->_data[i] = mapfunc(_data[i]);
  }
  return result;
}

template <typename T>
template <typename F>
List<T> *List<T>::filter_in_place(F filterfunc) {
  Int p = 0;
  for (Int i = 0; i < _len; i++) {
    if (filterfunc(_data[i]))
      _data[p++] = _data[i];
  }
  pop(_len - p);
  shrink(false);
  return this;
}

template <typename T>
template <typename F>
List<T> *List<T>::filter(F filterfunc) const {
  return clone()->filter_in_place(filterfunc);
}

template <typename T>
template <typename F>
void List<T>::iter(F iterfunc) {
  for (Int i = 0; i < _len; i++) {
    iterfunc(_data[i]);
  }
}

template <typename T>
template <typename F>
void List<T>::iter_indexed(F iterfunc) {
  for (Int i = 0; i < _len; i++) {
    iterfunc(i, _data[i]);
  }
}

template <typename T>
template <typename F>
List<T> *List<T>::sort_in_place(F cmpfunc) {
  T *in = (T *) GC_MALLOC(sizeof(T) * _len);
  T *out = (T *) GC_MALLOC(sizeof(T) * _len);
  memcpy(in, _data, sizeof(T) * _len);
  Int step = 1;
  while (step < _len) {
    for (Int i = 0; i < _len; i += step * 2) {
      Int p = i, l = i, r = i + step, lmax = l + step, rmax = r + step;
      if (rmax > _len)
        rmax = _len;
      if (lmax > _len)
        lmax = _len;
      while (l < lmax && r < rmax) {
        int c = cmpfunc(in[l], in[r]);
        if (c < 0) {
          out[p++] = in[l++];
        } else {
          out[p++] = in[r++];
        }
      }
      while (l < lmax) {
        out[p++] = in[l++];
      }
      while (r < rmax) {
        out[p++] = in[r++];
      }
    }
    T *tmp = in;
    in = out;
    out = tmp;
    step += step;
  }
  _data = in;
  _cap = _len;
  return this;
}

template <typename T>
template <typename F>
List<T> *List<T>::sort(F cmpfunc) const {
  return clone()->sort_in_place(cmpfunc);
}

template <typename T>
template <typename A, typename F>
A List<T>::fold(A init, F foldfunc) const {
  A result = init;
  for (Int i = 0; i < _len; i++) {
    result = foldfunc(result, _data[i]);
  }
  return result;
}

template <typename T>
bool List<T>::eq(List<T> *that) const {
  if (_len != that->_len)
    return false;
  for (Int i = 0; i < _len; i++)
    if (!_data[i]->eq(that->_data[i]))
      return false;
  return true;
}

template <typename T>
List<T> *List<T>::remove(Int start, Int count) {
  require(start >= 0 && count >= 0 && start + count <= _len,
    "index out of range");
  Int end = start + count;
  if (count <= 0) return 0;
  memmove(_data + start, _data + end, sizeof(T) * (_len - end));
  memset(_data + _len - count, 0, sizeof(T) * count);
  _len -= count;
  return shrink(false);
}

template <typename T>
List<T> *List<T>::remove(Int at) {
  return remove(at, 1);
}

template <typename T>
List<T> *List<T>::set_len(Int len) {
  if (len > _len) {
    expand(len);
    memset(_data + _len, 0, sizeof(T) * (len - _len));
    _len = len;
  } else if (len < _len) {
    memset(_data + len, 0, sizeof(T) * (_len - len));
    _len = len;
    return shrink(false);
  }
  return this;
}

template <typename T>
List<T> *List<T>::fill(Int start, Int count, T value) {
  require(start >= 0 && count >= 0 && start + count <= _len,
    "index out of range");
  Int end = start + count;
  for (Int i = start; i < end; i++) {
    _data[i] = value;
  }
  return this;
}

template <typename T>
List<T> *List<T>::subarr(Int start, Int count) const {
  require(start >= 0 && start + count <= _len, "index out of range");
  if (count <= 0)
    return new List<T>();
  return new List<T>(count, _data + start);
}

template <typename T>
T List<T>::shift() {
  require(_len > 0, "empty array");
  T result = _data[0];
  remove(0, 1);
  return result;
}
