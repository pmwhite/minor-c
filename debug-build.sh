#!/usr/bin/env sh

set -euxo pipefail

gcc -Wall -Wextra -Werror \
  -nostdlib \
  -z noexecstack \
  -fdata-sections \
  -fno-builtin \
  -std=c99 -pedantic \
  -g \
  src/main.S src/main.c -o src/main
