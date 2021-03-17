#!/bin/sh
set -e
cd "$(dirname "$0")"
test -x build.sh

../../tools/install.sh --autosetup

./configure
make -j4
