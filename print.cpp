#pragma once
#include "base/base.hpp"

#include <iostream>

// constexpr static isize print_buffer_size = 16 * mem_KiB;
// static byte print_buffer_memory[print_buffer_size];

std::ostream& operator<<(std::ostream& os, String s){
	os.write((char const*)raw_data(s), len(s));
	return os;
}

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

