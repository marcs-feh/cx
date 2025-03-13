#include "base/base.hpp"
#include <iostream>

constexpr isize mem_size = mem_KiB * 16;
static byte memory[mem_size];

int main(){
	Arena arena = {0};
	arena_init(&arena, Slice<byte>(memory, mem_size));

	auto allocator = arena_allocator(&arena);
	std::cout << arena.offset << '\n'; 
	{
		auto region = arena_region_begin(&arena);
		defer(arena_region_end(region));

		std::cout << make<i32>(allocator) << '\n';
		std::cout << make<i32>(allocator) << '\n';
		std::cout << make<i32>(allocator) << '\n';
		std::cout << make<i32>(allocator) << '\n';
		std::cout << make<i32>(allocator) << '\n';
		std::cout << make<i32>(allocator) << '\n';
		std::cout << arena.offset << '\n'; 
	}
	std::cout << arena.offset << '\n'; 

	String cu = "Sexooo";
}
