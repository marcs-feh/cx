#include "base.hpp"

void arena_init(Arena* a, Slice<byte> buf){
	a->data = (void*)raw_data(buf);
	a->offset = 0;
	a->capacity = len(buf);
	a->last_allocation = NULL;
	a->region_count = 0;
}

void* arena_alloc(Arena* a, isize size, isize align){
	if(size == 0){ return nullptr; }
	uintptr base = (uintptr)a->data;
	uintptr current = base + (uintptr)a->offset;

	isize available = a->capacity - (current - base);

	uintptr aligned  = mem_align_forward_ptr(current, align);
	uintptr padding  = aligned - current;
	isize required = padding + size;

	if(required > available){
		return nullptr; /* Out of memory */
	}

	a->offset += required;
	void* allocation = (void*)aligned;
	a->last_allocation = allocation;
	mem_set(allocation, 0, size);

	return allocation;
}

bool arena_resize_in_place(Arena* a, void* ptr, isize size){
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
	isize new_size,
	isize new_align,
	void* old_ptr,
	isize old_size,
	isize /* old_align */
){
	auto arena = (Arena*)data;
	Result<void*, AllocatorError> result{0};

	using M = AllocatorMode;
	using C = AllocatorCapability;

	switch(mode){
	case M::Alloc: {
		if(!mem_valid_alignment(new_align)){
			result.error = AllocatorError::BadAlignment;
			return result;
		}

		result.value = arena_alloc(arena, new_size, new_align);
		if(!result.value){
			result.error = AllocatorError::OutOfMemory;
		}
	} break;

	case M::Realloc: {
		if(old_ptr != nullptr && arena_resize_in_place(arena, old_ptr, new_size)){
			result.value = old_ptr;
		}
		else {
			result.value = arena_alloc(arena, new_size, new_align);
			if(result.value){
				mem_copy_no_overlap(result.value, old_ptr, old_size);
			}
			else {
				result.error = AllocatorError::OutOfMemory;
			}
		}
	} break;

	case M::Free: {
		result.error = AllocatorError::NotSupported;
	} break;

	case M::FreeAll: {
		arena_free_all(arena);
	} break;

	case M::Query: {
		u32 caps = u32(C::Alloc) | u32(C::FreeAll) | u32(C::Realloc);
		result.value = (void*)uintptr(caps);
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
