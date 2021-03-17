#pragma once

// MD5 cannot be considered a cryptographically secure algorithm anymore
// and shouldn't be used for this purpose. It is included here as a basic
// checksumming and RNG tool, e.g. to check if file contents have changed
// in a build system by comparing their checksums.

struct MD5Digest {
  Word32 a, b, c, d;
};

class MD5 : public PtrFreeGC {
private:
  MD5Digest _state;
  unsigned char fragment[64];
  Word _fragment_length;
  Word _total_length;
  bool _finished;
public:
  MD5Digest digest();
  Str *bytedigest();
  Str *hexdigest();
  static MD5Digest digest(Str *str);
  static Str *bytedigest(Str *str);
  static Str *hexdigest(Str *str);
  void reset() {
    _state.a = 0x67452301;
    _state.b = 0xefcdab89;
    _state.c = 0x98badcfe;
    _state.d = 0x10325476;
    _fragment_length = 0;
    _total_length = 0;
    _finished = false;
  }
  void update(const void *data, Int len);
  void update(Str *str) {
    update(str->c_str(), str->len());
  }
  void update_block(const unsigned char *data);
  void finish();
  MD5() {
    reset();
  }
};

class MD5Random : public PtrFreeGC {
private:
  MD5 _state;
  MD5Digest _digest;
  Int _seed;
  Word32 _counter;
  int _sel;
  Word32 next() {
    switch (_sel++) {
      case 0: {
        Word32 buf[] = { (Word32) _seed, (Word32) (_seed >> 32), _counter++ };
        _state.reset();
        _state.update(buf, sizeof(buf));
        _state.finish();
        _digest = _state.digest();
        return _digest.a;
      }
      case 1: return _digest.b;
      case 2: return _digest.c;
      case 3: _sel = 0; return _digest.d;
    }
    ensure(false, "unreachable code");
    return 0;
  }
public:
  MD5Random(Int seed = 0) : _seed(seed), _counter(0), _sel(0), _state() {}
  Int next_int(Int mod) {
    require(mod > 0, "argument must be positive");
    Word a = next();
    if (mod >= 0x10000) {
      a <<= 32;
      a += next();
    }
    return (Int) (a % mod);
  }
  Word next_word(Word mod) {
    require(mod > 0, "argument must be positive");
    Word a = next();
    if (mod >= 0x10000) {
      a <<= 32;
      a += next();
    }
    return a % mod;
  }
  Word next_word32() {
    return next();
  }
  Word next_bits(Int bits) {
    if (bits <= 32) {
      return next() & ((1 << bits) - 1);
    } else {
      Word a = next();
      a <<= 32;
      a += next();
      return a & ((1 << bits) - 1);
    }
  }
  bool next_bool() {
    return next() & 1;
  }
  double next_double() {
    return (double) next() / 4294967296.0;
  }
};
