#pragma once

template <typename A, typename B>
class Variant2 : public GC {
  enum Tag { tag_A, tag_B };
private:
  Tag _tag;
  union Data {
    A a;
    B b;
  } _data;
public:
  Variant2(A a) : _tag(tag_A) { _data.a = a; }
  Variant2(B b) : _tag(tag_B) { _data.b = b; }
  Tag tag() { return _tag; }
  bool is_a() { return _tag == tag_A; }
  bool is_b() { return _tag == tag_B; }
  A a() { return _data.a; }
  B b() { return _data.b; }
};

template <typename A, typename B, typename C>
class Variant3 : public GC {
  enum Tag { tag_A, tag_B, tag_C };
private:
  Tag _tag;
  union Data {
    A a;
    B b;
    C c;
  } _data;
public:
  Variant3(A a) : _tag(tag_A) { _data.a = a; }
  Variant3(B b) : _tag(tag_B) { _data.b = b; }
  Variant3(C c) : _tag(tag_C) { _data.c = c; }
  Tag tag() { return _tag; }
  bool is_a() { return _tag == tag_A; }
  bool is_b() { return _tag == tag_B; }
  bool is_c() { return _tag == tag_C; }
  A a() { return _data.a; }
  B b() { return _data.b; }
  C c() { return _data.c; }
};

template <typename A, typename B, typename C, typename D>
class Variant4 : public GC {
  enum Tag { tag_A, tag_B, tag_C, tag_D };
private:
  Tag _tag;
  union Data {
    A a;
    B b;
    C c;
    D d;
  } _data;
public:
  Variant4(A a) : _tag(tag_A) { _data.a = a; }
  Variant4(B b) : _tag(tag_B) { _data.b = b; }
  Variant4(C c) : _tag(tag_C) { _data.c = c; }
  Variant4(D d) : _tag(tag_D) { _data.d = d; }
  Tag tag() { return _tag; }
  bool is_a() { return _tag == tag_A; }
  bool is_b() { return _tag == tag_B; }
  bool is_c() { return _tag == tag_C; }
  bool is_d() { return _tag == tag_D; }
  A a() { return _data.a; }
  B b() { return _data.b; }
  C c() { return _data.c; }
  D d() { return _data.d; }
};

template <typename A, typename B, typename C, typename D, typename E>
class Variant5 : public GC {
  enum Tag { tag_A, tag_B, tag_C, tag_D, tag_E };
private:
  Tag _tag;
  union Data {
    A a;
    B b;
    C c;
    D d;
    E e;
  } _data;
public:
  Variant5(A a) : _tag(tag_A) { _data.a = a; }
  Variant5(B b) : _tag(tag_B) { _data.b = b; }
  Variant5(C c) : _tag(tag_C) { _data.c = c; }
  Variant5(D d) : _tag(tag_D) { _data.d = d; }
  Variant5(E e) : _tag(tag_E) { _data.e = e; }
  Tag tag() { return _tag; }
  bool is_a() { return _tag == tag_A; }
  bool is_b() { return _tag == tag_B; }
  bool is_c() { return _tag == tag_C; }
  bool is_d() { return _tag == tag_D; }
  bool is_e() { return _tag == tag_E; }
  A a() { return _data.a; }
  B b() { return _data.b; }
  C c() { return _data.c; }
  D d() { return _data.d; }
  E e() { return _data.e; }
};
