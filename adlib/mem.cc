#include "lib.h"

void *GC::operator new(size_t size) {
  return GC_MALLOC(size);
}

void *PtrFreeGC::operator new(size_t size) {
  void *p = GC_MALLOC_ATOMIC(size);
  memset(p, 0, size);
  return p;
}

namespace AdLib {

size_t num_persistent;
size_t cap_persistent;
void **persistent_ptrs;

void GCPersistPtr(void *ptr) {
  if (num_persistent == cap_persistent) {
    if (!persistent_ptrs) {
      cap_persistent = 1024;
      persistent_ptrs = (void **) GC_malloc(sizeof(void *) * cap_persistent);
      GC_add_roots(&persistent_ptrs, &persistent_ptrs + 1);
    } else {
      void **new_ptrs
          = (void **) GC_malloc(sizeof(void *) * cap_persistent * 2);
      for (size_t i = 0; i < cap_persistent; i++)
        new_ptrs[i] = persistent_ptrs[i];
      cap_persistent *= 2;
      persistent_ptrs = new_ptrs;
    }
  }
  persistent_ptrs[num_persistent++] = ptr;
}
}
