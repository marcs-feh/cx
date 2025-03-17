#!/usr/bin/env sh

buildMode="$1"
[ -z "$buildMode" ] && buildMode='debug'

cxx='clang++ -std=c++17'
cflags='-Wall -Wextra -fPIC -fno-strict-aliasing'
iflags='-I./base -I./deps/mimalloc/include'

strip=''
case "$buildMode" in
	'debug') cflags="$cflags -g -O0" ;;
	'release') cflags="$cflags -O3"; strip='yes' ;;
esac

Run(){ echo "* $@"; $@; }

set -eu

Run $cxx $cflags $iflags -o test.exe \
	main.cpp base/base.cpp \
	deps/mimalloc/mimalloc.a

[ $strip ] && Run strip test.exe

