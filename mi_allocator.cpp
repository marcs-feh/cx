#pragma once

#include "base.hpp"
#include "mimalloc.h"

// static thread_local auto mi_allocator_last_error = AllocatorError::None;
//
// Result<void*, AllocatorError> mi_heap_allocator_func(
// 	void*,
// 	AllocatorMode mode,
// 	isize size,
// 	isize align,
// 	void* old_ptr,
// 	isize old_size
// ){
// 	Result<void*, AllocatorError> result{0};
//
// 	using M = AllocatorMode;
// 	using C = AllocatorCapability;
//
// 	switch(mode){
// 	case AllocatorMode::Alloc: {
// 		result.value = mi_zalloc_aligned(size, align);
// 		if(!result.value){
// 			result.error = AllocatorError::OutOfMemory;
// 		}
// 	} break;
//
// 	case AllocatorMode::Realloc: /* Unsupported */ break;
//
// 	case AllocatorMode::Free: {
// 		mi_free(old_ptr);
// 	} break;
//
// 	case AllocatorMode::FreeAll: /* Unsupported */ break;
//
// 	case AllocatorMode::Query:
// 	break;
// 	}
//
// 	return result;
// }



