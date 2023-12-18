#!/usr/bin/env sh

cd $(dirname $0)
MAIN=$(pwd)/../src/main TEST_DIR=$(pwd) ./cram.py *.t
