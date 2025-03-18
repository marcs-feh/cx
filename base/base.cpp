#include "base.hpp"

#include "assert.cpp"
#include "memory.cpp"
#include "arena.cpp"
#include "utf8.cpp"
#include "strings.cpp"

#if defined(USE_MIMALLOC)
#include "mi_allocator.cpp"
#endif

