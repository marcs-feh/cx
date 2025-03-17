#!/usr/bin/env sh

cc='clang -std=c17'
cflags='-O3 -fPIC -fno-strict-aliasing'

set -xeu

$cc $cflags deps/mimalloc/src/static.c -I deps/mimalloc/include -c -o deps/mimalloc/mimalloc.o

