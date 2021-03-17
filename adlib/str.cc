#include "lib.h"

Str *Str::chomp() {
  if (_len > 0 && _data[_len - 1] == '\n')
    _len--;
  if (_len > 0 && _data[_len - 1] == '\r')
    _len--;
  _data[_len] = '\0';
  return this;
}

Str *Str::repeat(Int n) {
  Str *result = new Str(n * _len);
  for (Int i = 0; i < n; i++) {
    result->add(this);
  }
  return result;
}

StrList *Str::split(const char *s, Int n) {
  if (n == 1)
    return split(s[0]);
  List<Int> *parts = new List<Int>();
  parts->add(-n);
  for (Int i = 0; i < _len - n; i++) {
    if (_data[i] == s[0] && memcmp(_data + i, s, n)) {
      parts->add(i);
      i += n;
    }
  }
  parts->add(_len);
  StrList *result = new StrList(parts->len() - 1);
  for (Int i = 1; i < parts->len(); i++) {
    Int begin = parts->at(i - 1) + n;
    Int end = parts->at(i);
    result->add(new Str(_data + begin, end - begin));
  }
  return result;
}

StrList *Str::split(Str *sep) {
  return split(sep->_data, sep->_len);
}

static bool is_space(char ch) {
  switch (ch) {
    case ' ':
    case '\r':
    case '\n':
    case '\t':
    case '\v':
    case '\f':
      return true;
    default:
      return false;
  }
}

StrList *Str::split(char ch) {
  Int parts = 1;
  for (Int i = 0; i < _len; i++) {
    if (_data[i] == ch)
      parts++;
  }
  StrList *result = new StrList(parts);
  Int last = 0;
  for (Int i = 0; i < _len; i++) {
    if (_data[i] == ch) {
      result->add(new Str(_data + last, i - last));
      last = i + 1;
    }
  }
  result->add(new Str(_data + last, _len - last));
  return result;
}

StrList *Str::split_ws() {
  Int parts = 0;
  for (Int i = 1; i < _len; i++) {
    if (is_space(_data[i-1]) && !is_space(_data[i]))
      parts++;
  }
  StrList *result = new StrList(parts);
  Int start = 0;
  while (start < _len && is_space(_data[start]))
    start++;
  for (Int i = start+1;  i < _len; i++) {
    if (is_space(_data[i]) && !is_space(_data[i-1])) {
      result->add(new Str(_data + start, i - start));
      start = i;
      while (start < _len && is_space(_data[start]))
        start++;
      i = start;
    }
  }
  if (start != _len)
    result->add(new Str(_data + start, _len - start));
  return result;
}

Str *Str::remove(Int start, Int count) {
  Int end = start + count;
  require(start >= 0 && start <= _len, "index out of range");
  require(end <= _len, "index out of range");
  if (count <= 0) return this;
  memmove(_data + start, _data + end, _len - end);
  _len -= count;
  _data[_len] ='\0';
  return shrink(false);
}

Str *Str::remove(Int at) {
  return remove(at, 1);
}

Str *Str::set_len(Int len) {
  require (len >= 0, "invalid length");
  if (len > _len) {
    expand(len);
    memset(_data + _len, 0, len - _len + 1);
    _len = len;
  }
  else if (len < _len) {
    memset(_data + len, 0, _len - len);
    _len = len;
    return shrink(false);
  }
  return this;
}

StrList *Str::split(const char *s) {
  return split(s, strlen(s));
}

StrList *Str::split_lines() {
  StrList *result = split('\n');
  for (Int i = 0; i < result->len(); i++) {
    result->at(i)->chomp();
  }
  return result;
}

Str *StrJoin(const StrList *arr, const char *sep, Int n) {
  if (arr->len() == 0)
    return new Str();
  Int len = (arr->len() - 1) * n;
  for (Int i = 0; i < arr->len(); i++) {
    len += arr->at(i)->len();
  }
  Str *result = new Str(len);
  result->add(arr->first());
  for (Int i = 1; i < arr->len(); i++) {
    if (n >= 0)
      result->add(sep, n);
    result->add(arr->at(i));
  }
  return result;
}

Str *StrJoin(const StrList *arr, const char *sep) {
  return StrJoin(arr, sep, strlen(sep));
}

Str *StrJoin(const StrList *arr, char ch) {
  return StrJoin(arr, &ch, 1);
}

Str *StrJoin(const StrList *arr, const Str *sep) {
  return StrJoin(arr, sep->c_str(), sep->len());
}

bool Str::starts_with(const char *s, Int n) const {
  if (n > _len)
    return false;
  return memcmp(_data, s, n) == 0;
}

bool Str::starts_with(const char *s) const {
  return starts_with(s, strlen(s));
}

bool Str::starts_with(const Str *str) const {
  return starts_with(str->c_str(), str->len());
}

bool Str::ends_with(const char *s, Int n) const {
  if (n > _len)
    return false;
  return memcmp(_data + _len - n, s, n) == 0;
}

bool Str::ends_with(const char *s) const {
  return ends_with(s, strlen(s));
}

bool Str::ends_with(const Str *str) const {
  return ends_with(str->c_str(), str->len());
}

int Cmp(Str *str1, Str *str2) {
  Int len1 = str1->len();
  Int len2 = str2->len();
  int result = memcmp(str1->c_str(), str2->c_str(), Min(len1, len2));
  if (result == 0) {
    if (len1 < len2)
      result = -1;
    else if (len1 > len2)
      result = 1;
  }
  return result;
}

int StrCmp(Str *str1, Str *str2) {
  return Cmp(str1, str2);
}

bool Str::eq(const char *s, Int n) const {
  if (n != _len)
    return false;
  return memcmp(_data, s, n) == 0;
}

bool Str::eq(const char *s) const {
  return eq(s, strlen(s));
}

bool Str::eq(const Str *str) const {
  return eq(str->c_str(), str->len());
}

Str *Str::substr(Int start, Int count) {
  require(0 <= start && start < _len && count >= 0 && start + count <= _len,
    "index out of range");
  return new Str(_data + start, count);
}

Int Str::find(char ch, Int from) const {
  require(from <= _len, "index out of range");
  for (Int i = from; i < _len; i++) {
    if (_data[i] == ch)
      return i;
  }
  return NOWHERE;
}

const Int kmp_table_default_size = 256;

static Int KmpSearch(const char *needle, Int needle_len,
                      const char *haystack, Int haystack_len)
{
  Int fixed_table[kmp_table_default_size];
  Int *next;
  if (needle_len > kmp_table_default_size-1)
    next = (Int *) GC_malloc_atomic(sizeof(Int) * (needle_len+1));
  else
    next = fixed_table;
  memset(next, 0, sizeof(Int) * (needle_len+1));
  // build next table.
  next[0] = -1;
  Int pos = 1, target = 0;
  while (pos < needle_len) {
    while (target >= 0 && needle[pos] != needle[target])
      target = next[target];
    pos++;
    target++;
    if (needle[pos] == needle[target])
      next[pos] = next[target];
    else
      next[pos] = target;
  }
  // KMP search
  Int hpos = 0, npos = 0;
  while (hpos < haystack_len) {
    if (needle[npos] == haystack[hpos]) {
      npos++;
      hpos++;
      if (npos == needle_len) {
        return hpos - needle_len;
      }
    } else {
      npos = next[npos];
      if (npos < 0) {
        npos++;
        hpos++;
      }
    }
  }
  return -1;
}

static Int StrStr(const char *needle, Int needle_len,
                      const char *haystack, Int haystack_len)
{
  if (needle_len == 0)
    return 0;
  if (needle_len > haystack_len)
    return -1;
  if (needle_len == 1) {
    char ch = needle[0];
    for (Int i = 0; i < haystack_len-1; i++) {
      if (haystack[i] == ch)
        return i;
    }
    return -1;
  }
  if (needle_len <= 4) {
    Word32 pat = 0;
    Word32 cur = 0;
    Word32 mask = ((Word32) -1) >> (32-needle_len*8);
    for (Int i = 0; i < needle_len; i++) {
      pat <<= 8;
      cur <<= 8;
      pat += (unsigned char) needle[i];
      cur += (unsigned char) haystack[i];
    }
    if (pat == cur)
      return 0;
    for (Int i = needle_len; i < haystack_len; i++) {
      cur <<= 8;
      cur += (unsigned char) haystack[i];
      if ((cur & mask) == pat) {
        return i - needle_len + 1;
      }
    }
    return -1;
  }
  return KmpSearch(needle, needle_len, haystack, haystack_len);
}

Int Str::find(const char *s, Int n, Int from) const {
  const int mult_base = 3;
  require(n >= 0, "negative length");
  require(from < _len, "index out of range");
  Int off = StrStr(s, n, _data + from, _len - from);
  return off < 0 ? off : from + off;
}

Int Str::find(const char *s, Int from) const {
  return find(s, strlen(s), from);
}

Int Str::find(const Str *str, Int from) const {
  return find(str->_data, str->_len, from);
}

Int Str::rfind(char ch) const {
  for (Int i = _len - 1; i >= 0; i--) {
    if (_data[i] == ch)
      return i;
  }
  return NOWHERE;
}

Int Str::rfind(const char *s, Int n) const {
  require(n >= 0, "empty string");
  if (n == 0)
    return _len;
  if (n == 1)
    return rfind(s[0]);
  if (n >_len)
    return NOWHERE;
  Int end = _len - n;
  char ch = s[0];
  for (Int i = end - 1; i >= 0; i--) {
    if (memcmp(_data + i, s, n) == 0)
      return i;
  }
  return NOWHERE;
}

Int Str::rfind(const char *s) const {
  return rfind(s, strlen(s));
}

Int Str::rfind(const Str *str) const {
  return rfind(str->_data, str->_len);
}

Str *Str::replace_count(Int n, const Str *pattern, const Str *replacement) {
  require(pattern->len() > 0, "pattern must be non-empty");
  Str *result = new Str();
  Int from = 0;
  for (;;) {
    if (n-- <= 0) {
      result->add(_data + from, _len - from);
      break;
    }
    Int pos = find(pattern, from);
    if (pos < 0) {
      result->add(_data + from, _len - from);
      break;
    }
    result->add(_data + from, pos - from);
    result->add(replacement);
    from = pos + pattern->len();
  }
  return result;
}

Str *ToStr(Int x) {
  char buffer[sizeof(LongInt) * 4 + 1];
  sprintf(buffer, "%" LONG_FMT "d", (LongInt) x);
  return S(buffer);
}

Str *ToStr(Word x) {
  char buffer[sizeof(LongWord) * 4 + 1];
  sprintf(buffer, "%" LONG_FMT "u", (LongWord) x);
  return S(buffer);
}

Str *CStrToStr(const char *s) {
  return S(s);
}
