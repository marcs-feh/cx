#include "base.hpp"

void arena_init(Arena* a, Slice<byte> buf){
	a->data = (void*)raw_data(buf);
	a->offset = 0;
	a->capacity = len(buf);
	a->last_allocation = NULL;
	a->region_count = 0;
}

void* arena_alloc(Arena* a, isize size, isize align){
	uintptr base = (uintptr)a->data;
	uintptr current = base + (uintptr)a->offset;

	isize available = a->capacity - (current - base);

	uintptr aligned  = mem_align_forward_ptr(current, align);
	uintptr padding  = aligned - current;
	uintptr required = padding + size;

	if(required > available){
		return nullptr; /* Out of memory */
	}

	a->offset += required;
	void* allocation = (void*)aligned;
	a->last_allocation = allocation;
	mem_set(allocation, 0, size);

	return allocation;
}

bool arena_resize(Arena* a, void* ptr, isize size){
	uintptr base    = (uintptr)a->data;
	uintptr current = base + (uintptr)a->offset;
	uintptr limit   = base + a->capacity;

	ensure((uintptr)ptr >= base && (uintptr)ptr < limit, "Pointer is not owned by arena");

	if(ptr == a->last_allocation){
		isize last_allocation_size = current - (uintptr)a->last_allocation;
		if(((current - last_allocation_size) + size) > limit){
			return false; /* No space left */
		}

		a->offset += size - last_allocation_size;
		return true;
	}

	return false;
}

void arena_free_all(Arena* a){
	ensure(a->region_count == 0, "Arena has dangling regions");
	a->offset = 0;
	a->last_allocation = NULL;
}

ArenaRegion arena_region_begin(Arena* a){
	ArenaRegion reg = {
		.arena = a,
		.offset = a->offset,
	};
	a->region_count += 1;
	return reg;
}

void arena_region_end(ArenaRegion reg){
	ensure(reg.arena->region_count > 0, "Arena has an improper region counter");
	ensure(reg.arena->offset >= reg.offset, "Arena has a lower offset than region");

	reg.arena->offset = reg.offset;
	reg.arena->region_count -= 1;
}

Result<void*, AllocatorError> arena_allocator_func (
	void* data,
	AllocatorMode mode,
	isize size,
	isize align,
	void* old_ptr,
	isize old_size
){
	auto arena = (Arena*)data;
	Result<void*, AllocatorError> result{0};

	using M = AllocatorMode;
	using C = AllocatorCapability;

	if(!mem_valid_alignment(align)){
		result.error = AllocatorError::BadAlignment;
		return result;
	}

	switch(mode){
	case M::Alloc: {
		result.value = arena_alloc(arena, size, align);
		if(!result.value){
			result.error = AllocatorError::OutOfMemory;
		}
	} break;

	case M::Resize: {
		if(arena_resize(arena, old_ptr, size)){
			result.value = old_ptr;
		} else {
			result.error = AllocatorError::OutOfMemory;
		}
	} break;

	case M::Free: /* Unsupported */ break;

	case M::FreeAll: {
		arena_free_all(arena);
	} break;

	case M::Query: {
		u32 caps = u32(C::Alloc) | u32(C::FreeAll) | u32(C::Resize);
		result.value = (void*)uintptr(caps);
	} break;

	case M::LastError: {
		result.error = arena->last_error;
	} break;

	default: {
		result.error = AllocatorError::UnknownMode;
	} break;
	}

	arena->last_error = result.error;
	return result;
}

Allocator arena_allocator(Arena* arena){
	Allocator a = {
		.data = (void*)arena,
		.func = arena_allocator_func,
	};
	return a;
}
