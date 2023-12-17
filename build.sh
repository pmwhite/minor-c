#!/usr/bin/env sh

set -euxo pipefail

gcc -s -O2 -Wall -Wextra -Werror \
  -nostdlib \
  -z noexecstack \
  -Wl,--gc-sections \
  -fdata-sections \
  -fno-builtin \
  -std=c99 -pedantic \
  src/main.S src/main.c -o src/main
