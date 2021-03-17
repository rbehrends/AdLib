#!/usr/bin/env python

import sys
sys.dont_write_bytecode = True
sys.path.insert(0, "cnf")

import adlibtools

def options(opt):
  opt.add_option("--prefix", action="store", dest="prefix", default=".",
    help="installation prefix [default: '.']")
  opt.add_option("-q", "--quiet", action="store_true", dest="quiet",
    help="suppress build output")
  opt.add_option("--with-re2c", action="store_true", dest="use_re2c",
    help="use re2c")
  opt.add_option("--without-re2c", action="store_false", dest="use_re2c",
    help="do not use re2c")
  opt.add_option("--with-checks", action="store_true", dest="checks",
    help="enable contract checking (implied by --adlibdev)")
  opt.add_option("--with-tests", action="store_true", dest="tests",
    help="build tests (implied by --adlibdev)")
  opt.add_option("--with-boehm-gc", action="store_true", dest="boehm_gc",
    help="use Boehm GC")
  opt.add_option("--with-gc-path", action="store", dest="gc_path",
    help="path to boehm GC (optional)", default="", metavar="DIR")
  opt.add_option("--adlibdev", action="store_true", dest="adlibdev",
    help="Change build defaults for adlib development")
  opt.add_option("--debug", action="store_true", dest="debug",
    help="disable optimization for better debugging")
  opt.load("compiler_c")
  opt.load("compiler_cxx")
  adlibtools.hook("options", opt)

def configure(cnf):
  adlibtools.hook("preconf", cnf)
  cnf.env.USE_RE2C = cnf.options.use_re2c
  cnf.env.BOEHM_GC = cnf.options.boehm_gc
  cnf.env.GC_PATH = cnf.options.gc_path
  cnf.env.INCLUDES = cnf.env.INCLUDES + [ "." ]
  cnf.env.BUILD_TESTS = cnf.options.adlibdev or cnf.options.tests
  if cnf.options.boehm_gc:
    cnf.env.LIB = cnf.env.LIB + [ "gc" ]
    gc_path = cnf.options.gc_path
    if gc_path:
      cnf.env.LIBPATH = cnf.env.LIBPATH + [ gc_path + "/lib" ]
      cnf.env.INCLUDES = cnf.env.INCUDES + [ gc_path + "/include" ]
  cnf.load("compiler_c")
  cnf.load("compiler_cxx")
  cnf.check(header_name="stdlib.h", auto_add_header_name=True, mandatory=False)
  cnf.check(header_name="unistd.h", auto_add_header_name=True, mandatory=False)
  cnf.check(header_name="dirent.h", auto_add_header_name=True, mandatory=False)
  cnf.check(header_name="sys/types.h", auto_add_header_name=True,
    mandatory=False)
  cnf.define_cond("HAVE_ISATTY",
    cnf.check(features="c",
      fragment="#include <unistd.h>\nint main() { return(isatty(0)); }\n",
      msg="Checking for isatty()",
      mandatory = False))
  cnf.define_cond("USE_BOEHM_GC", cnf.options.boehm_gc)
  cnf.start_msg("Setting GC to")
  cnf.end_msg(cnf.options.boehm_gc and "boehm" or "tiny gc")
  if cnf.options.adlibdev:
    cnf.check_cpp_std("c++98 c++03")
  defs = { }
  defs["CHECKS"] = (cnf.options.checks or cnf.options.adlibdev) and "1" or "0"
  adlibtools.config_num_types(cnf, defs)
  adlibtools.hook("postconf", cnf)
  adlibtools.write_config(cnf, defs)
  adlibtools.hook("configure", cnf)

def build(bld):
  from glob import glob
  adlibtools.quiet(bld)

  if not bld.options.debug:
    bld.env.append_value("CFLAGS", "-O2")
    bld.env.append_value("CXXFLAGS", "-O2")
  bld.env.append_value("CFLAGS", "-g")
  bld.env.append_value("CXXFLAGS", "-g")

  adlibtools.hook("prebuild", bld)

  # Build library

  bld.adlib_library("adlib", extra = [ "gclib/gc.c" ])

  # Build program

  # bld.adlib_program("main", "src")

  adlibtools.hook("build", bld)

