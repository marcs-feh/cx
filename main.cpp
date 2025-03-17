#include "base/base.hpp"
#include "print.cpp"

constexpr isize mem_size = mem_KiB * 16;
static byte memory[mem_size];

int main(){
	Arena arena{};
	arena_init(&arena, Slice<byte>(memory, mem_size));
	auto allocator = arena_allocator(&arena);

	print("Arena:", arena.offset);
	auto arr = make_dynamic_array<f32>(allocator, 0);
	for(int i = 0; i < 20; i++){
		append(&arr, f32(i));
		print(arr);
		print("Arena:", arena.offset);
	}

	return 0;
}
