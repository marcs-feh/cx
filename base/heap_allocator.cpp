#include "base.hpp"

#include <stdlib.h>
#include <mimalloc.h>

thread_local AllocatorError heap_allocator_last_error = AllocatorError::None;

static inline
void* heap_alloc(isize size, isize align){
	ensure(mem_valid_alignment(align), "Invalid memory alignment");
	return mi_zalloc_aligned(size, align);
}

static inline
void heap_free(void* ptr){
	return mi_free(ptr);
}

static
Result<void*, AllocatorError> heap_allocator_func(
	void*,
	AllocatorMode mode,
	isize size,
	isize align,
	void* old_ptr,
	isize
){
	Result<void*, AllocatorError> result{0};

	using M = AllocatorMode;
	using C = AllocatorCapability;

	switch(mode){
	case AllocatorMode::Alloc: {
		result.value = heap_alloc(size, align);
		if(!result.value){
			result.error = AllocatorError::OutOfMemory;
		}
	} break;

	case AllocatorMode::Resize: /* Unsupported */ break;

	case AllocatorMode::Free: {
		heap_free(old_ptr);
	} break;

	case AllocatorMode::FreeAll: /* Unsupported */ break;

	case AllocatorMode::Query: {
		u32 caps = u32(C::Alloc) | u32(C::Free);
		result.value = (void*)uintptr(caps);
	} break;

	case AllocatorMode::LastError: {
		result.error = heap_allocator_last_error;
	} break;

	default: {
		result.error = AllocatorError::UnknownMode;
	} break;
	}

	heap_allocator_last_error = result.error;
	return result;
}

Allocator heap_allocator(){
	return {
		.data = nullptr,
		.func = heap_allocator_func,
	};
}

