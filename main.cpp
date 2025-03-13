#include "base/base.hpp"
#include <iostream>

template<typename T>
std::ostream& operator<<(std::ostream& os, Slice<T> s){
	std::cout << "[ ";
	for(isize i = 0; i < len(s); i++){
		std::cout << s[i] << ' ';
	}
	std::cout << ']';
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, DynamicArray<T> arr){
	std::cout << "(len: " << arr._length << ", cap: " << arr._capacity << ")[ ";
	for(isize i = 0; i < arr._length; i++){
		std::cout << arr._data[i] << ' ';
	}
	std::cout << ']';
	return os;
}

template<typename T>
void print(T x){
	std::cout << std::boolalpha << x << '\n';
}

template<typename T, typename ... Rest>
void print(T x, Rest&& ... rest){
	std::cout << std::boolalpha << x << ' ';
	print(rest...);
}

constexpr isize mem_size = mem_KiB * 16;
static byte memory[mem_size];

int main(){
	Arena arena = {0};
	arena_init(&arena, Slice<byte>(memory, mem_size));
	auto allocator = arena_allocator(&arena);

	auto [arr, err] = make_dynamic_array<i32>(allocator, 2);
	print(arr);
	append(&arr, 4);
	append(&arr, 2);
	append(&arr, 0);
	append(&arr, 6);
	append(&arr, 9);
	print(arr);
	remove(&arr, 1);
	print(arr);
	remove(&arr, 0);
	insert(&arr, 2, 4);
	print(arr);

	String cu = "Sexooo";
}
