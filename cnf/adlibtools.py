from waflib import Logs, Context, Configure
conf = Configure.conf

from os.path import join as mkpath

def load_wscript(path):
  dict = {}
  try:
    with open(path, "r") as file:
      code = file.read()
  except:
    code = ""
  exec(compile(code, path, "exec"), dict)
  return dict

wscript_setup = load_wscript("setup/wscript")
wscript_develop = load_wscript("develop/wscript")

def hook(funcname, ctx):
  if wscript_setup.has_key(funcname):
    wscript_setup[funcname](ctx)
  if wscript_develop.has_key(funcname):
    wscript_develop[funcname](ctx)

def as_list(l):
  if type(l) == type(""):
    return l.split()
  else:
    return l

def uniq(l):
  return list(set(l))

def split_off_tests(name, files):
  def istest(path):
    import os
    return os.path.basename(str(path)).startswith("test")
  tests = [ file for file in files if istest(file) ]
  src = [ file for file in files if not istest(file) ]
  return src, tests

def build_tests(bld, tests, libs):
  import os
  if bld.env.BUILD_TESTS or bld.options.tests:
    for test in tests:
      test = test.relpath()
      tgt = mkpath("test", os.path.splitext(test)[0])
      bld.program(features = "cxx c",
        target = tgt, source = test, use = libs)

def build_re2c(bld, srcdir, gen_in_tree, inst_in_tree, opts):
  import os
  if bld.env.USE_RE2C or bld.options.use_re2c:
    re2cfiles = bld.path.ant_glob(mkpath(srcdir, "*.re"))
  else:
    re2cfiles = []
  genfiles = []
  if re2cfiles:
    for refile in re2cfiles:
      ccfile = os.path.splitext(str(refile))[0] + ".cc"
      if gen_in_tree:
        ccfile = bld.path.make_node(ccfile)
        instpath = None
      else:
        ccfile = bld.bldnode.make_node(ccfile)
        if inst_in_tree:
          instpath = srcdir
        else:
          instpath = None
      genfiles.append(ccfile.relpath())
      bld(rule="re2c %s -o ${TGT} ${SRC}" % opts,
        source = refile, target = ccfile, install_path=instpath)
    if gen_in_tree: # otherwise, waf can't find the generated files
      bld.add_group()
  return genfiles

@conf
def check_cpp_std(cnf, standards):
  standards = as_list(standards)
  supported = None
  cnf.start_msg("Checking for C++ standard")
  for std in standards:
    try:
      cnf.check_cxx(cxxflags="-std=%s" % std, fragment="int main() { return 0; }")
      supported = std
      break
    except cnf.errors.ConfigurationError:
      pass
  if not supported:
    cnf.end_msg("implicit")
  else:
    cnf.end_msg(supported)
    cnf.env.append_value("CXXFLAGS", ["-std=%s" % supported])

@conf
def adlib_library(bld, name, **kw):
  extra = as_list(kw.get("extra", []))
  import os
  srcdir = kw.get("dir", name)
  files = bld.path.ant_glob(mkpath(srcdir, "*.cc"))
  files += bld.path.ant_glob(mkpath(srcdir, "*.cpp"))
  files += bld.path.ant_glob(mkpath(srcdir, "*.c"))
  files += extra
  src, tests = split_off_tests(name, files)
  libpath = mkpath("lib", name)
  bld.stlib(features = "cxx c",
    target = libpath, source = src)
  build_tests(bld, tests, [ libpath ])

@conf
def adlib_program(bld, name, srcdir, **kw):
  extra = as_list(kw.get("extra", []))
  libs = as_list(kw.get("libs", []))
  from glob import glob
  if not "adlib" in libs:
    libs.append("adlib")
  libs = [ mkpath("lib", lib) for lib in libs ]
  files = glob(mkpath(srcdir, "*.cc")) + glob(mkpath(srcdir, "*.c")) + extra
  src, tests = split_off_tests(name, files)
  gen = build_re2c(bld, srcdir,
    kw.get("gen_in_tree", False),
    kw.get("inst_in_tree", False),
    kw.get("re2c_opts", "--no-generation-date --no-version"))
  bld.program(features = "cxx c",
    target = mkpath("bin", name), source = uniq(src + gen), use = libs)
  build_tests(bld, tests, libs)

def interceptLogInfo(msg, *args, **kw):
  if msg.startswith("Waf: "):
    return
  if msg.startswith("%r finished successfully "):
    return
  oldLogInfo(msg, *args, **kw)

oldLogInfo = Logs.info
#Logs.info = interceptLogInfo

def try_call(module, funcname, *args, **kw):
  func = module.__dict__.get(funcname, None)
  if func:
    return func(*args, **kw)
  return None

def quiet(ctx):
  import logging
  if ctx.options.quiet:
      Logs.log.level = logging.ERROR

class silent:
  def __init__(self, conf):
    self.conf = conf
  def __enter__(self):
    def skip(*args, **kw): pass
    self.start_msg = self.conf.start_msg
    self.end_msg = self.conf.end_msg
    self.conf.start_msg = skip
    self.conf.end_msg = skip
    return self
  def __exit__(self, *args):
    self.conf.start_msg = self.start_msg
    self.conf.end_msg = self.end_msg

def write_template(cnf, target, source, defs, top=True):
  import sys, re, waflib
  def redef(m):
    return defs.get(m.group(0)[1:-1], "")
  try:
    cnf.start_msg("Generating '%s'" % target)
    contents = open(source).read()
    contents = re.sub("@.*?@", redef, contents)
    if top:
      # Root file in the "top" directory
      node = cnf.path.make_node(target)
    else:
      # Root file in the "build" directory.
      node = cnf.bldnode.make_node(target)
    node.parent.mkdir()
    node.write(contents)
    cnf.env.append_unique(waflib.Build.CFG_FILES, [node.abspath()])
    cnf.end_msg("ok")
  except:
    cnf.end_msg("failed", "RED")
    sys.exit(1)

def write_config(cnf, defs):
  import sys, waflib
  # Generate config headers
  defs["CONFIG_DEFS"] = cnf.get_config_header()
  write_template(cnf, "adlib/config.h", "cnf/config.h.in", defs,
    False)
  DEFKEYS = waflib.Tools.c_config.DEFKEYS
  for key in cnf.env[DEFKEYS]:
    cnf.undefine(key)
  cnf.env[DEFKEYS] = []
  # Generate Makefile
  try:
    configargs = sys.argv[sys.argv.index("configure") + 1:]
  except:
    configargs = sys.argv[2:]
  write_template(cnf, "Makefile", "cnf/Makefile.waf.in", {
    "CONFIGARGS" : " ".join(configargs)
  })

def config_num_types(cnf, defs):
  # Set lang to "c" to use the C compiler to check instead
  lang = "cxx"
  # Which system-specific types are supported?
  cnf.define_cond("HAVE_LONG_LONG",
    cnf.check(features=lang, fragment="long long x;",
      msg="Checking for 'long long' type",
      mandatory = False))
  cnf.define_cond("HAVE_LONG_DOUBLE",
    cnf.check(features=lang, fragment="long double x;",
      msg="Checking for 'long double' type",
      mandatory = False))
  cnf.define_cond("HAVE_OFF_T",
    cnf.check(features=lang,
      fragment="#include <sys/types.h>\noff_t x;\n",
      msg="Checking for 'off_t' type",
      mandatory = False))
  # Figure out sizes of integral types for systems
  # that don't support stdint.h
  # The following dictionary contains initial guesses
  # to make tests quicker on typical 64-bit and 32-bit
  # systems.
  guesses = {
    "char": [ 1 ],
    "short": [ 2, 4 ],
    "int": [ 4, 2 ],
    "long": [ 8, 4 ],
    "void *": [ 8, 4 ],
    "long long": [ 8, 4 ],
    "float": [ 4, 8 ],
    "double": [ 8, 16 ],
    "long double": [ 16, 8 ],
    "off_t": [ 8, 4, 16 ],
  }
  # Types that require extra includes
  type_incl = {
    "off_t": "#include <sys/types.h>\n",
  }
  # csize will later contain a mapping of types to sizes.
  csize = { }
  # List of all sizes we are going to check
  all_sizes = set([1, 2, 4, 8, 16, 32 ])
  # Check whether a condition is true.
  def check_cond(cond, prefix=""):
    code = "%s\nstatic int x[(%s) ? 1 : -1 ] = { 1 };" % (prefix, cond)
    return cnf.check(features=lang, fragment=code, mandatory=False)
  # Figure out sizes for any given type
  # Suppress log messages for the duration.
  with silent(cnf) as rep:
    for t in [ "void *",
      "char", "short", "int", "long", "long long",
      "float", "double",
      "off_t",
    ]:
      guess = guesses.get(t, [])
      other = list(all_sizes.difference(set(guess)))
      other.sort()
      for i in guess + other:
        if check_cond("sizeof(%s) == %d" % (t, i), type_incl.get(t, "")):
          csize[t] = i
          rep.start_msg("Checking for sizeof(%s)" % t)
          rep.end_msg(str(i))
          cnf.define(("SIZEOF_%s" % t).upper().replace(" ", "_").replace("*", "P"), i)
          break
  # Figure out types for Int<n> and Word<n>
  maxintsize = 1
  for i in [ 1, 2, 4, 8 ]:
    bits = i * 8
    for t in ["char", "short", "int", "long", "long long"]:
      if csize.get(t, 0) == i:
        cnf.define("SIZEOF_WORD%d" % bits, csize[t])
        cnf.define("SIZEOF_INT%d" % bits, csize[t])
        defs["Word%d" % bits] = "unsigned %s" % t
        defs["Int%d" % bits] = "signed %s" % t
        maxintsize = i
        break
  # Are we running on a system with no 64-bit integers?
  cnf.define_cond("HAVE_64BIT_SUPPORT", maxintsize == 8)
  if maxintsize < 8:
    cnf.define("SIZEOF_WORD64", 4)
    cnf.define("SIZEOF_INT64", 4)
  # Generate types and sizes for a pointer-sized integral type
  for t in ["char", "short", "int", "long", "long long"]:
    k = csize.get(t, 0)
    if k >= csize["void *"]:
      cnf.define("SIZEOF_WORD", k)
      # SIZEOF_INT is already in use for "int", so we use
      # SIZEOF_INTEGER for "Int"
      cnf.define("SIZEOF_INTEGER", k)
      defs["Word"] = "unsigned %s" % t
      defs["Int"] = "signed %s" % t
      break
  # Define portable "biggest" integral types
  if cnf.have_define("HAVE_LONG_LONG"):
    defs["LongWord"] = "unsigned long long"
    defs["LongInt"] = "signed long long"
    defs["LONG_FMT"] = "ll"
    defs["Offset"] = "signed long long"
  else:
    defs["LongWord"] = "unsigned long"
    defs["LongInt"] = "signed long"
    defs["LONG_FMT"] = "ll"
    defs["Offset"] = "signed long"
  # Find size and type of `off_t` that can be used prior to
  # including <sys/types.h>
  for t in ["char", "short", "int", "long", "long long"]:
    k = csize.get(t, 0)
    if k >= csize["off_t"]:
      cnf.define("SIZEOF_OFFSET", k)
      defs["Offset"] = "signed %s" % t
      break
  # Generate portable printf() formats for Int and Word types.
  if csize["long"] == csize["void *"]:
    defs["WORD_FMT"] = "l"
  elif cnf.have_define("HAVE_LONG_LONG") and \
      csize["long long"]==csize["void *"]:
    defs["WORD_FMT"] = "ll"
  else:
    defs["WORD_FMT"] = ""
