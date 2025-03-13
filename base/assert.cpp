#include "base.hpp"

#include <stdio.h>

[[noreturn]]
void panic(char const * msg){
	fprintf(stderr, "Panic: %s\n", msg);
	__builtin_trap();
}

void ensure(bool pred, char const * msg){
	[[unlikely]] if(!pred){
		fprintf(stderr, "Assertion Failed: %s\n", msg);
		__builtin_trap();
	}
}

#ifndef DISABLE_BOUNDS_CHECK
void bounds_check_assert(bool pred, char const * msg){
	[[unlikely]] if(!pred){
		fprintf(stderr, "Bounds check error: %s\n", msg);
		__builtin_trap();
	}
}
#else
void bounds_check_assert(bool, char const *){}
#endif
