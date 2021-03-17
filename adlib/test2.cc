#include "lib.h"
#include "set.h"

void Main() {
  StrList *arr
      = ReadProcessLines(new Str("seq"), new StrList(1, new Str("10000")));
  Check(arr->len() == 10001, "read process output as lines");
  Word hash = 0;
  for (StrList::Each it(arr); it; ++it) {
    hash += Hash(*it);
  }
  Check((hash & 0xffff) == 0xc756, "hashing strings");
  Str *data = ReadFile("/dev/null");
  Str *thisfile = S(__FILE__);
  if (thisfile->starts_with("../")) {
    thisfile = thisfile->range_excl(3, thisfile->len());
  }
  Opt<FileInfo *> finfo = FileStat(thisfile);
  Check(finfo && finfo.get()->is_file && finfo.get()->size > 0, "file stat");
  Check(data && data->len() == 0, "read bytes from file");
  arr = ReadLines(thisfile);
  Check(arr && arr->len() > __LINE__, "read lines from file");
  Check(WriteFile("/dev/null", S("test")), "write bytes to file");
  static CStr args[] = { "17" };
  Check(System(S("exit"), StrArrLit(args)) == 17, "invoke shell");
  Check(ListFiles(".")->len() > 0, "reading directories");
  StrSet *files = new StrSet(ListFiles("adlib"));
  files = new StrSet(ListFileTree(CurrentDir(), ListFilesRelative));
  Check(files->contains(thisfile), "reading directories recursively");
}
