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

void* mem_alloc(Allocator a, isize size, isize align){
	return a.func(a.data, AllocatorMode::Alloc, size, align, nullptr, 0).value;
}

void* mem_resize(Allocator a, void* ptr, isize old_size, isize new_size){
	return a.func(a.data, AllocatorMode::Resize, new_size, 0, ptr, old_size).value;
}

void* mem_realloc(Allocator a, void* ptr, isize old_size, isize new_size, isize align){
	void* p = a.func(a.data, AllocatorMode::Resize, new_size, align, ptr, old_size).value;
	if(!p){
		p = mem_alloc(a, new_size, align);
		if(p){
			mem_free(a, ptr, old_size);
		}
	}
	return p;
}

void mem_free(Allocator a, void* ptr, isize size){
	a.func(a.data, AllocatorMode::Free, 0, 0, ptr, size);
}

void mem_free_all(Allocator a){
	a.func(a.data, AllocatorMode::FreeAll, 0, 0, nullptr, 0);
}

u32 mem_query(Allocator a){
	auto val = uintptr(a.func(a.data, AllocatorMode::Query, 0, 0, nullptr, 0).value);
	return u32(val);
}

AllocatorError mem_last_error(Allocator a){
	return a.func(a.data, AllocatorMode::LastError, 0, 0, nullptr, 0).error;
}

