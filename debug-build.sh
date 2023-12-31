#!/usr/bin/env bash

set -euxo pipefail

gcc -Wall -Wextra -Werror \
  -nostdlib \
  -z noexecstack \
  -fdata-sections \
  -fno-builtin \
  -std=c99 -pedantic \
  -g \
  src/main.S src/main.c -o src/main

test/run_tests.sh

src/main translate self-hosted/main.minc > self-hosted/main.c 2> self-hosted/main.stderr
