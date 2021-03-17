#pragma once

class GC {
public:
  void *operator new(size_t size);
  void operator delete(void *p) {
  }
};

class PtrFreeGC {
public:
  void *operator new(size_t size);
  void operator delete(void *p) {
  }
};

template <typename T>
void GCVar(T &var) {
#ifndef USE_BOEHM_GC
  T *start = &var;
  T *end = start + 1;
  GC_add_roots(start, end);
#endif
}

template <typename T>
void GCVar(T &var, T val) {
#ifndef USE_BOEHM_GC
  T *start = &var;
  T *end = start + 1;
  GC_add_roots(start, end);
#endif
  var = val;
}

namespace AdLib {
  void GCPersistPtr(void * ptr);
}

template <typename T>
T *Persist(T* ref) {
  AdLib::GCPersistPtr(ref);
  return ref;
}
