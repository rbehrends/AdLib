#include "lib.h"

int ArgC;
char **ArgV;
Str *ProgName;
StrList *Args;

namespace AdLib {
  Initializer *initializers[MAX_INIT_PRIO+2];

  void InitSystem() {
    for (int i = 0; i < MAX_INIT_PRIO+2; i++) {
      for (Initializer *init = initializers[i]; init; init = init->next) {
        init->init();
      }
    }
  }
}

extern void Main();

void *run_with_gc(void *arg) {
  AdLib::InitSystem();
  ProgName = Persist(S(ArgV[0]));
  Args = Persist(new StrList(ArgC - 1));
  for (int i = 1; i < ArgC; i++)
    Args->add(new Str(ArgV[i]));
  Main();
  return NULL;
}

static void *dummy[1] = { NULL };

int main(int argc, char **argv) {
  GC_INIT();
  // We need at least one global variable for Tiny GC to work.
  GCVar(dummy);
  ArgC = argc;
  ArgV = argv;
  GC_call_with_gc_active(run_with_gc, NULL);
  return 0;
}
