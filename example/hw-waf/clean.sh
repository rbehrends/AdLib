#!/bin/sh
set -e
cd "$(dirname "$0")"
test -x clean.sh
rm -rf adlib cnf gclib tools configure auto.def wscript
rm -rf bin build config.log Makefile
