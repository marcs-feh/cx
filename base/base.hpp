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

		bounds_check_assert(from >= 0 && from < _length && to >= 0 && to <= _length && from <= to, "Index to sub-slice is out of bounds");

		Slice<T> s;
		s._length = to - from;
		s._data = &_data[from];
		return s;
	}

	Slice(){}
	Slice(T* ptr, isize len) : _data{ptr}, _length{len}{}
};

template<typename T>
isize len(Slice<T> s){ return s._length; }

template<typename T>
T* raw_data(Slice<T> s){ return s._data; }

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
	Resize        = 1, // Resize an allocation in-place (zero filled)
	Free          = 2, // Mark allocation as free
	FreeAll       = 3, // Mark all allocations as free
	Query         = 4, // Query allocator's capabilities
	LastError     = 5, // Get last error emmited by allocator
};

enum struct AllocatorCapability : u8 {
	Alloc         = 1 << u8(AllocatorMode::Alloc),
	Resize        = 1 << u8(AllocatorMode::Resize),
	Free          = 1 << u8(AllocatorMode::Free),
	FreeAll       = 1 << u8(AllocatorMode::FreeAll),
	/* Query      = Always Available */
	/* LastError  = Always Available */
};

enum struct AllocatorError : u8 {
	None         = 0,
	OutOfMemory  = 1,
	BadAlignment = 2,
	BadArgument  = 3,
	UnknownMode  = 4,
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

void* mem_alloc(Allocator a, isize size, isize align);

void* mem_resize(Allocator a, void* ptr, isize old_size, isize new_size);

void mem_free(Allocator a, void* ptr, isize size);

void mem_free_all(Allocator a);

u32 mem_query(Allocator a);

AllocatorError mem_last_error(Allocator a);

template<typename T>
T* make(Allocator a){
	auto p = (T*)mem_alloc(a, sizeof(T), alignof(T));
	return p;
}

template<typename T>
Slice<T> make(Allocator a, isize n){
	auto p = (T*)mem_alloc(a, sizeof(T) * n, alignof(T));
	return Slice<T>(p, p ? n : 0);
}

//// String
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
};

static inline
isize len(String s){ return s._length; }

static inline
byte const* raw_data(String s){ return s._data; }

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

bool arena_resize(Arena* arena, void* ptr, isize size);

void arena_free_all(Arena* arena);

ArenaRegion arena_region_begin(Arena* a);

void arena_region_end(ArenaRegion reg);

Allocator arena_allocator(Arena* arena);

