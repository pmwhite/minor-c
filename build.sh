#!/usr/bin/env sh

set -euxo pipefail

gcc -s -O2 -Wall -Werror \
  -nostdlib \
  -z noexecstack \
  -Wl,--gc-sections \
  -fdata-sections \
  -fno-builtin \
  -std=c89 -pedantic \
  main.S main.c -o main
