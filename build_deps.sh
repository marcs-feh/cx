#!/usr/bin/env sh

BuildMimalloc(){
	local cc='clang -std=c17'
	local ar=ar
	local cflags='-O3 -fPIC -fno-strict-aliasing'

	cd ./deps

	set -eu

	$cc $cflags mimalloc/src/static.c -I mimalloc/include -c -o mimalloc/mimalloc.o
	$ar rcs mimalloc/mimalloc.a 
	rm mimalloc/mimalloc.o
}
