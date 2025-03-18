#include "base/base.hpp"
// #include "print.cpp"

constexpr isize mem_size = mem_KiB * 16;
static byte memory[mem_size];

int main(){
	// Arena arena{};
	// arena_init(&arena, Slice<byte>(memory, mem_size));
	// auto allocator = arena_allocator(&arena);

	auto allocator = heap_allocator();
	auto arr = make_dynamic_array<f32>(allocator, 0);
	defer(destroy(arr));
	for(int i = 0; i < 20; i++){
		append(&arr, f32(i));
		// print(arr);
	}

	return 0;
}
