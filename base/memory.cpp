#include "base.hpp"

void mem_set(void* p, byte val, isize count){
	__builtin_memset(p, val, count);
}

void mem_copy(void* dest, void const * src, isize count){
	__builtin_memmove(dest, src, count);
}

void mem_copy_no_overlap(void* dest, void const * src, isize count){
	__builtin_memcpy(dest, src, count);
}

i32 mem_compare(void const * a, void const * b, isize count){
	return __builtin_memcmp(a, b, count);
}

Result<void*, AllocatorError> mem_alloc(Allocator a, isize size, isize align){
	return a.func(a.data, AllocatorMode::Alloc, size, align, nullptr, 0, 0);
}

Result<void*, AllocatorError> mem_realloc(Allocator a, void* ptr, isize old_size, isize old_align, isize new_size, isize new_align){
	return a.func(a.data, AllocatorMode::Realloc, new_size, new_align, ptr, old_size, old_align);
}

AllocatorError mem_free(Allocator a, void* ptr, isize size, isize align){
	return a.func(a.data, AllocatorMode::Free, 0, 0, ptr, size, align).error;
}

AllocatorError mem_free_all(Allocator a){
	return a.func(a.data, AllocatorMode::FreeAll, 0, 0, nullptr, 0, 0).error;
}

u32 mem_query(Allocator a){
	auto val = uintptr(a.func(a.data, AllocatorMode::Query, 0, 0, nullptr, 0, 0).value);
	return u32(val);
}

