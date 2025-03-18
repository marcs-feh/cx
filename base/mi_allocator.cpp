#pragma once

#include "base.hpp"
// #include "mimalloc.h"

extern "C" {
	[[nodiscard]] void* mi_zalloc_aligned(size_t size, size_t alignment) noexcept;
	[[nodiscard]] void* mi_realloc_aligned(void* p, size_t newsize, size_t alignment) noexcept;
	void mi_free_aligned(void* p, size_t alignment) noexcept;
}

static
Result<void*, AllocatorError> mi_heap_allocator_func(
	void*,
	AllocatorMode mode,
	isize size,
	isize align,
	void* old_ptr,
	isize,
	isize old_align
){
	Result<void*, AllocatorError> result{0};

	using M = AllocatorMode;
	using C = AllocatorCapability;

	switch(mode){
	case M::Alloc: {
		result.value = mi_zalloc_aligned(size, align);
		if(!result.value){
			result.error = AllocatorError::OutOfMemory;
		}
	} break;

	case M::Realloc: {
		result.value = mi_realloc_aligned(old_ptr, size, align);
		if(!result.value){
			result.error = AllocatorError::OutOfMemory;
		}
	} break;

	case M::Free: {
		mi_free_aligned(old_ptr, old_align);
	} break;

	case M::FreeAll: {
		result.error = AllocatorError::NotSupported;
	}break;

	case M::Query:{
		u32 caps = u32(C::Alloc) | u32(C::Free) | u32(C::Realloc);
		result.value = (void*)uintptr(caps);
	} break;

	default: {
		result.error = AllocatorError::UnknownMode;
	} break;
	}

	return result;
}

Allocator heap_allocator(){
	return {
		.data = nullptr,
		.func = mi_heap_allocator_func,
	};
}
