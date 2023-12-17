#!/usr/bin/env sh

set -euxo pipefail

gcc -Wall -Wextra -Werror \
  -nostdlib \
  -z noexecstack \
  -fdata-sections \
  -fno-builtin \
  -std=c99 -pedantic \
  -g \
  main.S main.c -o main
