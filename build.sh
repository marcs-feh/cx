#!/usr/bin/env sh

buildMode="$1"
[ -z "$buildMode" ] && buildMode='debug'

cxx='clang++ -std=c++17'
cflags='-Wall -Wextra -fPIC -fno-strict-aliasing -fno-exceptions -fno-asynchronous-unwind-tables'
iflags='-I./base -I./deps/mimalloc/include'
ar='llvm-ar'

case "$buildMode" in
	'debug') cflags="$cflags -g -O0 -fsanitize=address" ;;
	'release') cflags="$cflags -s -O3 -static";
esac

Run(){ echo "* $@"; $@; }

BuildMimalloc(){
	local cc='clang -std=c17'
	local cflags='-O3 -fPIC -fno-strict-aliasing'

	cd ./deps
	Run $cc $cflags mimalloc/src/static.c -I mimalloc/include -c -o mimalloc/mimalloc.o
	Run $ar rcs mimalloc/mimalloc.a 
	Run rm mimalloc/mimalloc.o
	cd ..
}


set -eu

[ -f "deps/mimalloc/mimalloc.a" ] || BuildMimalloc

Run $cxx $cflags $iflags -o test.exe \
	main.cpp base/base.cpp \
	deps/mimalloc/mimalloc.a

