#!/bin/sh

set -xe

cd "$(dirname "$0")"

make clean
make -j
./main 2> failed_tests.log
make clean
