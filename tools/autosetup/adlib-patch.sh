#!/bin/sh
PATCH_FIND_TCLSH='
/^for tclsh/, /done/ { next }
/^for cc/ {
  print "echo \"Bootstrapping standalone Tcl interpreter\" 1>&2";
  sub(":-cc", ""); sub("gcc", "gcc clang cc"); print;
  next;
}
/^echo .*tclsh/ { next }
//
'

PATCH_CC_TCL='
/^\s*foreach tool/ {
  print;
  print "\t\tmsg-checking \"Checking for $tool...\"";
  next;
}
/^\s*define \$TOOL \$exe/ {
  print "\t\t\tmsg-result $exe";
  print;
  next;
}
/\{CFLAGS "-g -O2"\}/ {
  sub("\\{CFLAGS \"-g -O2\"\\}", "CFLAGS");
  print;
  next;
}
/^define CCACHE / {
  sub("ccache", "\"\"");
  print;
  next;
}
/^\s*define CXX .*get-define/ {
  print "\tdefine CXX [find-an-executable [get-define cross]g++ [get-define cross]clang++ [get-define cross]c++ false]";
  next;
}
/^\s*set try .*get-define/ {
  print "\tset try [list [get-define cross]gcc [get-define cross]clang [get-define cross]cc]";
  next;
}
/^define CC_FOR_BUILD / {
  print "define CC_FOR_BUILD [find-an-executable [get-env CC_FOR_BUILD \"\"] gcc clang cc false]";
  next;
}
//
'

PATCH_AUTOSETUP='
/--debug/ {
  sub("--debug", "--debug-autosetup");
  print;
  next;
}
/debug\s+=>/ {
  sub("debug", "debug-autosetup");
  print;
  next;
}
/opt-bool debug/ {
  sub("opt-bool debug", "opt-bool debug-autosetup");
  print;
  next;
}
//
'

FIND_TCLSH=autosetup/autosetup-find-tclsh
CC_TCL=autosetup/cc.tcl
AUTOSETUP=autosetup/autosetup

awk "$PATCH_FIND_TCLSH" <$FIND_TCLSH >$FIND_TCLSH.tmp \
   && mv -f $FIND_TCLSH.tmp $FIND_TCLSH && chmod 755 $FIND_TCLSH
awk "$PATCH_AUTOSETUP" <$AUTOSETUP >$AUTOSETUP.tmp \
   && mv -f $AUTOSETUP.tmp $AUTOSETUP && chmod 755 $AUTOSETUP
awk "$PATCH_CC_TCL" < $CC_TCL >$CC_TCL.tmp && mv -f $CC_TCL.tmp $CC_TCL
mv -f configure autosetup/configure
cat >configure <<'EOF'
#!/bin/sh
. $(dirname $0)/autosetup/configure
EOF
chmod 755 configure
