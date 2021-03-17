#pragma once

class Str;

typedef List<Str *> StrList;

class Str : public GC {
private:
  Int _len;
  Int _cap;
  char *_data;
  void resize(Int newcap) {
    char *newdata = (char *) GC_MALLOC_ATOMIC(newcap + 1);
    memcpy(newdata, _data, _len);
    _data = newdata;
    _cap = newcap;
  }
  void init(Int cap) {
    _len = 0;
    _cap = cap;
    // atomic memory is not zeroed by default.
    _data = (char *) GC_MALLOC_ATOMIC(cap + 1);
    _data[0] = '\0';
  }
  void init(const char *s, Int len) {
    _len = _cap = len;
    // atomic memory is not zeroed by default.
    _data = (char *) GC_MALLOC_ATOMIC(_cap + 1);
    memcpy(_data, s, len);
    _data[len] = '\0';
  }

public:
  class Each {
  private:
    const Str *_str;
    Int _i;

  public:
    Each(const Str *str, Int i = 0) {
      _str = str;
      _i = i;
    }
    operator bool() const {
      return _i < _str->_len;
    }
    void operator++() {
      _i++;
    }
    void operator++(int dummy) {
      _i++;
    }
    char &operator*() const {
      return _str->_data[_i];
    }
    char operator->() const {
      return _str->_data[_i];
    }
  };
  class Items : public GC {
    const Str *_str;
  public:
    Items(const Str *str) : _str(str) { }
    Each begin() const {
      return Each(_str, 0);
    }
    Each end() const {
      return Each(_str, _str->_len);
    }
  };
  class Range : public GC {
    const Str *_str;
    const Int _from, _to;
  public:
    Range(const Str *str, Int from, Int to)
        : _str(str)
        , _from(from)
        , _to(to) {
      require(from >= 0 && from <= str->_len, "invalid start of range");
      require(to >= 0 && to <= str-> _len, "invalid end of range");
    }
    Each begin() {
      return Each(_str, _from);
    }
    Each end() {
      return Each(_str, _to);
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
  Str(const char *s) {
    init(s, strlen(s));
  }
  Str(const char *s, Int n) {
    init(s, n);
  }
  Str(Int size) {
    init(size);
  }
  Str() {
    init(sizeof(Int) * 2 - 1);
  }
  Str(const Str *str) {
    init(str->_data, str->_len);
  }
  Str *clone() const {
    return new Str(this);
  }
  Str *expand(Int newlen) {
    if (newlen > _cap)
      resize(nextPow2(newlen));
    return this;
  }
  Str *shrink(bool fit = true) {
    if (_len != _cap)
      resize(_len);
    return this;
  }
  Str *add(char ch) {
    expand(_len + 1);
    _data[_len++] = ch;
    _data[_len] = 0;
    return this;
  }
  Str *add(const char *s, Int n) {
    expand(_len + n);
    memcpy(_data + _len, s, n);
    _len += n;
    _data[_len] = 0;
    return this;
  }
  Str *add(const Str *other) {
    return add(other->_data, other->_len);
  }
  Str *add(const char *s) {
    return add(s, strlen(s));
  }
  Str *remove(Int start, Int count);
  Str *remove(Int at);
  Str *set_len(Int len);
  Str *substr(Int start, Int count);
  Str *range_incl(Int start, Int end) {
    return substr(start, end - start + 1);
  }
  Str *range_excl(Int start, Int end) {
    return substr(start, end - start);
  }
  Str *chomp();
  Int find(const Str *str, Int from = 0) const;
  Int find(char ch, Int from = 0) const;
  Int find(const char *s, Int from = 0) const;
  Int find(const char *s, Int n, Int from) const;
  Int rfind(const Str *str) const;
  Int rfind(char ch) const;
  Int rfind(const char *s) const;
  Int rfind(const char *s, Int n) const;
  Str *replace_count(Int n, const Str *pattern, const Str *replacement);
  Str *replace_all(const Str *pattern, const Str *replacement) {
    return replace_count(_len, pattern, replacement);
  }
  StrList *split(Str *sep);
  StrList *split(char ch);
  StrList *split(const char *s);
  StrList *split(const char *s, Int n);
  StrList *split_lines();
  StrList *split_ws();
  Str *repeat(Int n);
  bool starts_with(const Str *str) const;
  bool starts_with(const char *s) const;
  bool starts_with(const char *s, Int n) const;
  bool ends_with(const Str *str) const;
  bool ends_with(const char *s) const;
  bool ends_with(const char *s, Int n) const;
  bool eq(const Str *str) const;
  bool eq(const char *str) const;
  bool eq(const char *str, Int n) const;
  Int len() const {
    return _len;
  }
  Int count() const {
    return _len;
  }
  char &ch(Int i) const {
    require(0 <= i && i <= _len, "string index out of range");
    return _data[i];
  }
  unsigned char &byte(Int i) const {
    require(0 <= i && i <= _len, "string index out of range");
    return ((unsigned char *)_data)[i];
  }
  char &at(Int i) const {
    require(0 <= i && i <= _len, "string index out of range");
    return _data[i];
  }
  char &operator[](Int i) const {
    require(0 <= i && i <= _len, "string index out of range");
    return _data[i];
  }
  char *c_str() const {
    return _data;
  }
  unsigned char *u_str() const {
    return (unsigned char *) _data;
  }
};

Str *StrJoin(const StrList *arr, const Str *sep);
Str *StrJoin(const StrList *arr, char ch);
Str *StrJoin(const StrList *arr, const char *sep);
Str *StrJoin(const StrList *arr, const char *sep, Int n);

#define StrArrLit(a) (new StrList(NUMOF(a), a, CStrToStr))

int Cmp(Str *str1, Str *str2);
int StrCmp(Str *str1, Str *str2);
Str *ToStr(Int x);
Str *ToStr(Int x);
Str *CStrToStr(const char *s);

static inline Str *S(const char *str) {
  return new Str(str);
}

static inline Str *S(Int i) {
  return ToStr(i);
}

static inline Str *S(Word w) {
  return ToStr(w);
}

static inline Str *S(const StrList *arr, const char *sep = " ") {
  return StrJoin(arr, sep);
}

static inline StrList *L() {
  return new StrList();
}

static inline StrList *L(const char *s) {
  return (new Str(s))->split_ws();
}

static inline StrList *L(const char *s, const char * sep){
  return (new Str(s))->split(sep);
}
