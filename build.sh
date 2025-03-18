#!/usr/bin/env sh

buildMode="$1"
[ -z "$buildMode" ] && buildMode='debug'

configFlags='-DUSE_MIMALLOC=1'

cxx='clang++ -std=c++17'
cflags='-Wall -Wextra -fPIC -fno-strict-aliasing -fno-exceptions -fno-asynchronous-unwind-tables -static-libgcc'
iflags='-I./base -I./deps/mimalloc/include'
ldflags=''

ar='llvm-ar'

case "$buildMode" in
	'debug')
		cflags="$cflags -g -O0 -fsanitize=address"
	;;
	'release')
		cflags="$cflags -s -O3"
	;;
esac

Run(){ echo "* $@"; $@; }

BuildMimalloc(){
	local cc='clang -std=c17'
	local cflags='-O3 -fPIC -fno-strict-aliasing -DNDEBUG'

	cd ./deps
	Run $cc $cflags mimalloc/src/static.c -I mimalloc/include -c -o mimalloc/mimalloc.o
	Run $ar rcs mimalloc/libmimalloc.a 
	cd ..
}

set -eu

[ -f "deps/mimalloc/mimalloc.o" ] || BuildMimalloc

cflags="$cflags $configFlags"

Run $cxx $cflags -o test.exe \
	deps/mimalloc/mimalloc.o base/base.cpp main.cpp \
	$ldflags


