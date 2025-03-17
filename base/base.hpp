#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <atomic>

//// Basic
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using uint = unsigned int;
using byte = uint8_t;
using rune = i32;

using isize = ptrdiff_t;

using uintptr = uintptr_t;

using f32 = float;
using f64 = double;

constexpr isize cache_line_size = 64;

template<typename T>
using Atomic = std::atomic<T>;

template<typename A, typename B = A>
struct Pair {
	A a;
	B b;
};

template<typename T>
T abs(T x){
	return (x < static_cast<T>(0)) ? - x : x;
}

template<typename T>
T min(T a, T b){ return a < b ? a : b; }

template<typename T, typename ...Args>
T min(T a, T b, Args... rest){
	if(a < b)
		return min(a, rest...);
	else
		return min(b, rest...);
}

template<typename T>
T max(T a, T b){
	return a > b ? a : b;
}

template<typename T, typename ...Args>
T max(T a, T b, Args... rest){
	if(a > b)
		return max(a, rest...);
	else
		return max(b, rest...);
}

template<typename T>
T clamp(T lo, T x, T hi){
	return min(max(lo, x), hi);
}

static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Bad float size");
static_assert(sizeof(isize) == sizeof(isize), "Mismatched (i/u)size");
static_assert(sizeof(void(*)(void)) == sizeof(void*), "Function pointers and data pointers must be of the same width");
static_assert(sizeof(void(*)(void)) == sizeof(uintptr), "Mismatched pointer types");

[[noreturn]]
void panic(char const * msg);

void ensure(bool pred, char const * msg);

void bounds_check_assert(bool pred, char const * msg);

// Same as a Pair, just with explicit naming for error handling.
template<typename Value, typename Error>
struct Result {
	Value value{};
	Error error{0};
};

// Used for error handling, this assumes that the no-error value of a Error enum is always 0.
template<typename ErrorEnum>
bool ok(ErrorEnum v){
	return i32(v) == 0;
}

template<typename Value, typename Error>
bool ok(Result<Value, Error> v){
	return i32(v.error) == 0;
}

template<typename T>
struct Slice {
	T*    _data   = nullptr;
	isize _length = 0;

	T& operator[](isize idx) {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	T const& operator[](isize idx) const {
		bounds_check_assert(idx >= 0 && idx < _length, "Index to slice is out of bounds");
		return _data[idx];
	}

	Slice<T> operator[](Pair<isize> range){
		isize from = range.a;
		isize to = range.b;

		bounds_check_assert(from >= 0 && from <= _length && to >= 0 && to <= _length && from <= to, "Index to sub-slice is ill-formed");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	Slice(){}
	Slice(T* ptr, isize len) : _data{ptr}, _length{len}{}
};

template<typename T> constexpr
auto len(Slice<T> const& s){ return s._length; }

template<typename T> constexpr
auto raw_data(Slice<T> const& s){ return s._data; }

//// Defer
namespace impl_defer {
	template<typename F>
	struct Deferred {
		F f;
		explicit Deferred(F&& f) : f(static_cast<F&&>(f)){}
		~Deferred(){ f(); }
	};
	template<typename F>
	auto make_deferred(F&& f){
		return Deferred<F>(static_cast<F&&>(f));
	}

#define _impl_defer_concat0(X, Y) X##Y
#define _impl_defer_concat1(X, Y) _impl_defer_concat0(X, Y)
#define _impl_defer_concat_counter(X) _impl_defer_concat1(X, __COUNTER__)
#define defer(Stmt) auto _impl_defer_concat_counter(_defer_) = ::impl_defer::make_deferred([&](){ do { Stmt ; } while(0); return; })
}

//// Memory
enum struct AllocatorMode : u8 {
	Alloc         = 0, // Allocate a chunk of memory (zero filled)
	Realloc       = 1, // Resize an allocation (zero filled)
	Free          = 2, // Mark allocation as free
	FreeAll       = 3, // Mark all allocations as free
	Query         = 4, // Query allocator's capabilities
};

enum struct AllocatorCapability : u8 {
	Alloc         = 1 << u8(AllocatorMode::Alloc),
	Realloc       = 1 << u8(AllocatorMode::Realloc),
	Free          = 1 << u8(AllocatorMode::Free),
	FreeAll       = 1 << u8(AllocatorMode::FreeAll),
	/* Query      = Always Available */
};

enum struct AllocatorError : u8 {
	None         = 0,
	OutOfMemory  = 1,
	BadAlignment = 2,
	BadArgument  = 3,
	NotSupported = 4,
	UnknownMode  = 5,
};

constexpr isize mem_KiB = 1024ll;
constexpr isize mem_MiB = mem_KiB * 1024ll;
constexpr isize mem_GiB = mem_MiB * 1024ll;

void mem_set(void* p, byte val, isize count);

void mem_copy(void* dest, void const * src, isize count);

void mem_copy_no_overlap(void* dest, void const * src, isize count);

i32 mem_compare(void const * a, void const * b, isize count);

template<typename Integer>
bool mem_valid_alignment(Integer a){
	return ((a & (a - 1)) == 0) && (a > 0);
}

static inline
uintptr mem_align_forward_ptr(uintptr p, uintptr a){
	ensure(mem_valid_alignment(a), "invalid memory alignment");
	uintptr mod = p & (a - 1); // fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

static inline
isize mem_align_forward_size(isize p, isize a){
	ensure(mem_valid_alignment(a), "Invalid size alignment");
	isize mod = p & (a - 1); // Fast modulo for powers of 2
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

using AllocatorFunc = Result<void*, AllocatorError>(*)(
	void* data,
	AllocatorMode mode,
	isize size,
	isize align,
	void* old_ptr,
	isize old_size
);

struct Allocator {
	void*          data;
	AllocatorFunc  func;
};

[[nodiscard]]
Result<void*, AllocatorError> mem_alloc(Allocator a, isize size, isize align);

[[nodiscard]]
Result<void*, AllocatorError> mem_resize(Allocator a, void* ptr, isize old_size, isize new_size);

AllocatorError mem_free(Allocator a, void* ptr, isize size, isize align);

AllocatorError mem_free_all(Allocator a);

u32 mem_query(Allocator a);

[[nodiscard]]
Result<void*, AllocatorError> mem_realloc(Allocator a, void* ptr, isize old_size, isize new_size, isize align);

template<typename T>
T* make(Allocator a){
	auto p = (T*)mem_alloc(a, sizeof(T), alignof(T)).value;
	return p;
}

template<typename T>
void destroy(T* ptr, Allocator a){
	mem_free(a, ptr, sizeof(T));
}

template<typename T>
Slice<T> make(isize n, Allocator a){
	auto p = (T*)mem_alloc(a, sizeof(T) * n, alignof(T)).value;
	return Slice<T>(p, p ? n : 0);
}

template<typename T>
void destroy(Slice<T> s, Allocator a){
	mem_free(a, (void*)raw_data(s), sizeof(T) * len(s));
}

//// Arena
struct Arena {
	void* data;
	isize offset;
	isize capacity;
	void* last_allocation;
	i32 region_count;
	AllocatorError last_error;
};

struct ArenaRegion {
	Arena* arena;
	isize offset;
};

void arena_init(Arena* a, Slice<byte> buf);

void* arena_alloc(Arena* arena, isize size, isize align);

bool arena_resize_in_place(Arena* arena, void* ptr, isize size);

void arena_free_all(Arena* arena);

ArenaRegion arena_region_begin(Arena* a);

void arena_region_end(ArenaRegion reg);

Allocator arena_allocator(Arena* arena);

//// Dynamic Array
constexpr isize dynamic_array_default_capacity = 16;

template<typename T>
struct DynamicArray {
	T*        _data;
	isize     _capacity;
	isize     _length;
	Allocator _allocator;

	T& operator[](isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds access to dynamic array");
		return _data[idx];
	}

	T const& operator[](isize idx) const{
		bounds_check_assert(idx >= 0 && idx < _length, "Out of bounds access to dynamic array");
		return _data[idx];
	}

	Slice<T> operator[](Pair<isize> range){
		isize from = range.a;
		isize to = range.b;

		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}
};

template<typename T>
Result<DynamicArray<T>, AllocatorError> make_dynamic_array(Allocator allocator, isize cap = dynamic_array_default_capacity){
	auto data = make<T>(cap, allocator);
	if(len(data) == 0){
		return {.error = AllocatorError::OutOfMemory};
	}

	DynamicArray<T> arr;
	arr._allocator = allocator;
	arr._length    = 0;
	arr._capacity  = cap;
	arr._data      = raw_data(data);

	return {.value = arr};
}

template<typename T>
void destroy(DynamicArray<T> arr){
	mem_free(arr->_allocator, arr->_data, sizeof(T) * arr->_capacity);
}

template<typename T>
void clear(DynamicArray<T>* arr){
	arr->_length = 0;
}

template<typename T>
void destroy(DynamicArray<T>* arr){
	mem_free(arr->_allocator, arr->_data, arr->_capacity * sizeof(T));
	arr->_capacity = 0;
}

template<typename T>
bool append(DynamicArray<T>* arr, T elem){
	if(arr->_length >= arr->_capacity){
		isize new_cap = max(arr->_length * 2, dynamic_array_default_capacity);
		auto new_data = mem_realloc(arr->_allocator, arr->_data, arr->_capacity * sizeof(T), new_cap, alignof(T));
		if(!new_data){
			return false;
		}
		arr->_capacity = new_cap;
		arr->_data = (T*)new_data;
	}
	arr->_data[arr->_length] = elem;
	arr->_length += 1;
	return true;
}

template<typename T>
bool pop(DynamicArray<T>* arr){
	if(arr->_length < 1){ return false; }
	arr->_length -= 1;
	auto v = arr->_data[arr->_length];
	return v;
}

template<typename T>
bool insert(DynamicArray<T>* arr, isize idx, T elem){
	bounds_check_assert(idx >= 0 && idx <= arr->_length, "Out of bounds index to insert_swap");
	if(idx == arr->_length){ return append(arr, elem); }

	bool ok = append(arr, elem);
	if(!ok){ return false; }

	isize nbytes = sizeof(T) * (arr->_length - 1 - idx);
	mem_copy(&arr->_data[idx + 1], &arr->_data[idx], nbytes);
	arr->_data[idx] = elem;
	return true;
}

template<typename T>
void remove(DynamicArray<T>* arr, isize idx){
	bounds_check_assert(idx >= 0 && idx < arr->_length, "Out of bounds index to remove");
	isize nbytes = sizeof(T) * (arr->_length - idx + 1);
	mem_copy(&arr->_data[idx], &arr->_data[idx+1], nbytes);
	arr->_length -= 1;
}

template<typename T>
bool insert_swap(DynamicArray<T>* arr, isize idx, T elem){
	bounds_check_assert(idx >= 0 && idx <= arr->_length, "Out of bounds index to insert_swap");
	if(idx == arr->_length){ return append(arr, elem); }

	bool ok = append(arr, arr->_data[idx]);
	[[unlikely]] if(!ok){ return false; }
	arr->_data[idx] = elem;

	return true;
}

template<typename T>
void remove_swap(DynamicArray<T>* arr, isize idx){
	bounds_check_assert(idx >= 0 && idx < arr->_length, "Out of bounds index to remove_swap");
	T last = arr->_data[arr->_length - 1];
	arr->_data[idx] = last;
	arr->_length -= 1;
}

template<typename T> constexpr
auto len(DynamicArray<T> const& a) { return a._length; }

template<typename T> constexpr
auto cap(DynamicArray<T> const& a) { return a._capacity; }

template<typename T> constexpr
auto allocator(DynamicArray<T> const& a) { return a._allocator; }

template<typename T> constexpr
auto slice(DynamicArray<T> a) { return a[{0, a._length}]; }

//// Array
template<typename T, int N>
struct Array {
	T v[N];

	T& operator[](isize idx){
		bounds_check_assert(idx >= 0 && idx < N, "Index to slice is out of bounds");
		return v[idx];
	}

	T const& operator[](isize idx) const {
		bounds_check_assert(idx >= 0 && idx < N, "Index to slice is out of bounds");
		return v[idx];
	}

	static_assert(N >= 0, "Array length must be greater than 0");
};

template<typename T, int N> constexpr
isize len(Array<T, N>){ return N; }

#include "internal_array_overloads.gen.cpp"

//// UTF-8
struct UTF8DecodeResult {
	rune codepoint;
	i32 size;
};

struct UTF8EncodeResult {
	Array<byte, 4> data;
	i32 size;
};

static inline
bool utf8_is_continuation(rune c){
	static const rune CONTINUATION1 = 0x80;
	static const rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}


UTF8EncodeResult utf8_encode(rune r);

UTF8DecodeResult utf8_decode(Slice<byte> s);

struct UTF8Iterator {
	Slice<byte> data;
	isize current;

	// WARNING: C++ Iterator insanity. C++ has some of the most shit
	// iterator design in any programming language and it should not be
	// taken seriously. This is *ONLY* to be able to iterate strings nicely.
	void operator++(){
		this->current += 1;
		while(this->current < len(this->data) && utf8_is_continuation(this->data._data[this->current])){
			this->current += 1;
		}
	}
	void operator--(){
		this->current -= 1;
		while(this->current > 0 && utf8_is_continuation(this->data._data[this->current])){
			this->current -= 1;
		}
	}
	rune operator*(){
		return utf8_decode(this->data[ {this->current, len(this->data)} ]).codepoint;
	}
	bool operator!=(UTF8Iterator const& it){
		return &raw_data(this->data)[this->current] != &raw_data(it.data)[it.current];
	}
	UTF8Iterator begin(){ return {data, 0}; }
	UTF8Iterator end(){ return {data, len(data)}; }
};

constexpr rune ERROR_RUNE = 0xfffd;

constexpr UTF8EncodeResult ERROR_RUNE_UTF8 = {
	.data = {0xef, 0xbf, 0xbd},
	.size = 0,
};

bool iter_done(UTF8Iterator* it);

UTF8DecodeResult iter_advance(UTF8Iterator* it);

UTF8DecodeResult iter_rewind(UTF8Iterator* it);

//// Strings
static inline
isize cstring_len(char const* cstr){
	isize size = 0;
	for(isize i = 0; cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

struct String {
	byte const* _data;
	isize _length;

	String() : _data{0}, _length{0} {}
	String(const char* cs) : _data{(byte const*)cs}, _length{cstring_len(cs)}{}
	explicit String(byte const* p, isize n) : _data{p}, _length{n}{}
	explicit String(Slice<byte> s) : _data{raw_data(s)}, _length{len(s)}{}

	byte operator[](isize idx){
		bounds_check_assert(idx >= 0 && idx < _length, "Index to string is out of bounds");
		return _data[idx];
	}

	String operator[](Pair<isize> range) const noexcept {
		isize from = range.a;
		isize to   = range.b;
		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-string is out of bounds");

		return String(&_data[from], to - from);
	}

	bool operator==(String lhs) const noexcept {
		if(lhs._length != _length){ return false; }
		return mem_compare(_data, lhs._data, _length) == 0;
	}

	bool operator!=(String lhs) const noexcept {
		if(lhs._length != _length){ return false; }
		return mem_compare(_data, lhs._data, _length) != 0;
	}

	// C++ Iterator bs

	UTF8Iterator begin(){ return {Slice((byte*)_data, _length), 0}; }
	UTF8Iterator end(){ return {Slice((byte*)_data, _length), _length}; }
};

static inline
isize len(String s){ return s._length; }

static inline
byte const* raw_data(String s){ return s._data; }

String str_clone(String s, Allocator a);

String str_concat(String s0, String s1, Allocator a);

isize str_rune_count(String s);

bool str_starts_with(String s, String prefix);

bool str_ends_with(String s, String suffix);

// String str_trim(String s, String cutset) ;
//
// String str_trim_leading(String s, String cutset) ;
//
// String str_trim_trailing(String s, String cutset) ;

isize str_find(String s, String pattern, isize start);

