#include "base.hpp"

constexpr isize max_cutset_len = 64;


UTF8Iterator str_iterator(String s) {
	UTF8Iterator it = {
		.data = Slice((byte*)raw_data(s), len(s)),
		.current = 0,
	};
	return it;
}

UTF8Iterator str_iterator_reversed(String s) {
	UTF8Iterator it = {
		.data = Slice((byte*)raw_data(s), len(s)),
		.current = len(s),
	};
	return it;
}

String str_clone(String s, Allocator a){
	auto buf = make<byte>(len(s) + 1, a);
	[[unlikely]] if(len(buf) == 0){ return ""; }
	buf[len(buf) - 1] = 0;
	mem_copy_no_overlap(raw_data(buf), raw_data(s), len(s));
	return String(buf[ {0, len(buf) - 1} ]);
}

[[nodiscard]]
String str_concat(String s0, String s1, Allocator a){
	auto buf = make<byte>(len(s0) + len(s1) + 1, a);

	[[unlikely]] if(len(buf) == 0){ return ""; }
	auto data = raw_data(buf);
	mem_copy_no_overlap(&data[0], raw_data(s0), len(s0));
	mem_copy_no_overlap(&data[len(s0)], raw_data(s1), len(s1));
	buf[len(buf) - 1] = 0;
	return String(buf[ {0, len(buf) - 1} ]);
}

isize str_rune_count(String s) {
	auto it = str_iterator(s);
	isize size = 0;

	for(auto it = str_iterator(s); !iter_done(it); iter_next(&it)){
		size += 1;
	}
	return size;
}

bool str_starts_with(String s, String prefix){
	if(len(prefix) == 0){ return true; }
	if(len(prefix) > len(s)){ return false; }
	i32 cmp = mem_compare(raw_data(s), raw_data(prefix), len(prefix));
	return cmp == 0;
}


bool str_ends_with(String s, String suffix){
	if(len(suffix) == 0){ return true; }
	if(len(suffix) > len(s)){ return false; }
	i32 cmp = mem_compare(raw_data(s) + len(s) - len(suffix), raw_data(suffix), len(suffix));
	return cmp == 0;
}


String str_trim(String s, String cutset) {
	String trimmed = str_trim_trailing(str_trim_leading(s, cutset), cutset);
	return trimmed;
}


String str_trim_leading(String s, String cutset) {
	ensure(len(cutset) <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	rune set[max_cutset_len] = {0};
	isize set_len = 0;
	isize cut_after = 0;

	/* Decode cutset */ {
		rune c; i32 n;

		isize i = 0;
		for(auto it = str_iterator(cutset);
			!iter_done(it) && i < max_cutset_len;
			c = iter_next(&it)
		){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		UTF8DecodeResult decoded = {0};

		for(auto it = str_iterator(s);
			!iter_done(it);
			decoded = iter_next_pair(&it)
		){
			bool to_be_cut = false;
			for(isize i = 0; i < set_len; i += 1){
				if(set[i] == decoded.codepoint){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_after += decoded.size;
			}
			else {
				break; // Reached first rune that isn't in cutset
			}

		}
	}

	return s[ {cut_after, len(s)} ];
}

String str_trim_trailing(String s, String cutset) {
	ensure(len(cutset) <= max_cutset_len, "Cutset string exceeds max_cutset_len");

	rune set[max_cutset_len] = {0};
	isize set_len = 0;
	isize cut_until = len(s);

	/* Decode cutset */ {
		rune c = 0;

		isize i = 0;
		for(auto it = str_iterator(cutset);
			!iter_done(it) && i < max_cutset_len;
			c = iter_next(&it)
		){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip cutset */ {
		UTF8DecodeResult decoded{0};
		for(auto it = str_iterator_reversed(s);
			!iter_done(it);
			decoded = iter_prev_pair(&it)
		){
			bool to_be_cut = false;
			for(isize i = 0; i < set_len; i += 1){
				if(set[i] == decoded.codepoint){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_until -= decoded.size;
			}
			else {
				break; // Reached first rune that isn't in cutset
			}
		}
	}

	return s[ {0, cut_until } ];
}

isize str_find(String s, String pattern, isize start){
	bounds_check_assert(start < len(s), "Cannot begin searching after string length");
	if(len(pattern) > len(s)){ return -1; }
	else if(len(pattern) == 0){ return start; }

	auto source_p  = raw_data(s);
	auto pattern_p = raw_data(pattern);

	auto length = len(s) - len(pattern);

	for(isize i = start; i < length; i++){
		if(mem_compare(&source_p[i], pattern_p, len(pattern)) == 0){
			return i;
		}
	}

	return -1;
}
