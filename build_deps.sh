#!/usr/bin/env sh

cc='clang -std=c17'
ar=ar
cflags='-O3 -fPIC -fno-strict-aliasing'

cd ./deps

set -xeu

$cc $cflags mimalloc/src/static.c -I mimalloc/include -c -o mimalloc/mimalloc.o
$ar rcs mimalloc/mimalloc.a 
rm mimalloc/mimalloc.o

