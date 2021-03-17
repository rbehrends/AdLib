#include "lib.h"
#include "md5.h"

static void add_hexword(Str *str, Word32 w) {
  static char hexdigits[] = "0123456789abcdef";
  char buf[8];
  for (int i = 0; i < 4; i++) {
    buf[2*i] = hexdigits[(w >> 4) & 0xf];
    buf[2*i+1] = hexdigits[w &0xf];
    w >>= 8;
  }
  str->add(buf, sizeof(buf));
}

static void add_word(Str *str, Word32 w) {
  char buf[4];
  for (int i = 0; i < 4; i++) {
    buf[i] = w & 0xff;
    w >>= 8;
  }
  str->add(buf, sizeof(buf));
}

MD5Digest MD5::digest() {
  return _state;
}

Str *MD5::hexdigest() {
  Str *result = new Str(32);
  finish();
  add_hexword(result, _state.a);
  add_hexword(result, _state.b);
  add_hexword(result, _state.c);
  add_hexword(result, _state.d);
  return result;
}

Str *MD5::bytedigest() {
  Str *result = new Str(16);
  finish();
  add_word(result, _state.a);
  add_word(result, _state.b);
  add_word(result, _state.c);
  add_word(result, _state.d);
  return result;
}

MD5Digest MD5::digest(Str *str) {
  MD5 digest;
  digest.update(str);
  return digest._state;
}

Str *MD5::hexdigest(Str *str) {
  MD5 digest;
  digest.update(str);
  return digest.hexdigest();
}

Str *MD5::bytedigest(Str *str) {
  MD5 digest;
  digest.update(str);
  return digest.bytedigest();
}

void MD5::update(const void *data, Int len) {
  require(len >= 0, "negative length");
  const unsigned char *ptr = (const unsigned char *) data;
  _total_length += len;
  if (len + _fragment_length < sizeof(fragment)) {
    memcpy(fragment + _fragment_length, ptr, len);
    _fragment_length += len;
    return;
  }
  if (_fragment_length != 0) {
    memcpy(fragment + _fragment_length, ptr, 64 - _fragment_length);
    update_block(fragment);
    len -= (64 - _fragment_length);
    ptr += (64 - _fragment_length);
    _fragment_length = 0;
  }
  while (len >= 64) {
    update_block(ptr);
    ptr += 64;
    len -= 64;
  }
  memcpy(fragment, ptr, len);
  _fragment_length = len;
}

void MD5::finish() {
  if (_finished)
    return;
  fragment[_fragment_length++] = 0x80;
  if (_fragment_length > 56) {
    memset(fragment + _fragment_length, 0, 64 - _fragment_length);
    update_block(fragment);
    _fragment_length = 0;
  }
  memset(fragment + _fragment_length, 0, 56 - _fragment_length);
  unsigned char * p = fragment + 56;
  Word n = _total_length * 8;
  for (int i = 0; i < 8; i++) {
    *p++ = (unsigned char) n;
    n >>= 8;
  }
  update_block(fragment);
  _finished = true;
}

void MD5::update_block(const unsigned char *data) {
  Word32 buf[16];
  Word32 a = _state.a;
  Word32 b = _state.b;
  Word32 c = _state.c;
  Word32 d = _state.d;
  for (int i = 0; i < 16; i++) {
    Word32 t = *data++;
    t |= (*data++) << 8;
    t |= (*data++) << 16;
    t |= (*data++) << 24;
    buf[i] = t;
  }
#include "md5block.h"
  _state.a += a;
  _state.b += b;
  _state.c += c;
  _state.d += d;
}
