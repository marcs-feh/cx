#include "base/base.hpp"
#include "print.cpp"

constexpr isize mem_size = mem_KiB * 16;
static byte memory[mem_size];

int main(){
	Arena arena = {0};
	arena_init(&arena, Slice<byte>(memory, mem_size));
	auto allocator = arena_allocator(&arena);

	String cu = "S → exooλ";
	UTF8Iterator it = {
		.data = Slice((byte*)raw_data(cu), len(cu)),
		.current = len(cu),
	};
	// UTF8DecodeResult res;

	// for(auto it = cu.begin(); it != cu.end(); ++it){
	for(auto it = cu.end() ;;){
		auto [c, n] = iter_rewind(&it);
		if(n == 0){ break; }

		auto enc = utf8_encode(c);
		char buf[5] = {0};
		mem_copy(buf, &enc.data, enc.size);
		print(buf, c);
	}

}
