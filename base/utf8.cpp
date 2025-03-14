#include "base.hpp"

constexpr i32 RANGE1 = 0x7f;
constexpr i32 RANGE2 = 0x7ff;
constexpr i32 RANGE3 = 0xffff;
constexpr i32 RANGE4 = 0x10ffff;

constexpr i32 UTF16_SURROGATE1 = 0xd800;
constexpr i32 UTF16_SURROGATE2 = 0xdfff;

constexpr i32 MASK2 = 0x1f; /* 0001_1111 */
constexpr i32 MASK3 = 0x0f; /* 0000_1111 */
constexpr i32 MASK4 = 0x07; /* 0000_0111 */

constexpr i32 MASKX = 0x3f; /* 0011_1111 */

constexpr i32 SIZE2 = 0xc0; /* 110x_xxxx */
constexpr i32 SIZE3 = 0xe0; /* 1110_xxxx */
constexpr i32 SIZE4 = 0xf0; /* 1111_0xxx */

constexpr i32 CONT = 0x80; /* 10xx_xxxx */

static inline
bool is_continuation_byte(rune c){
	static const rune CONTINUATION1 = 0x80;
	static const rune CONTINUATION2 = 0xbf;
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

bool iter_done(UTF8Iterator it){
	return it.current < 0 || it.current >= len(it.data);
}

UTF8EncodeResult utf8_encode(rune c){
	UTF8EncodeResult res = {};

	if(is_continuation_byte(c) ||
	   (c >= UTF16_SURROGATE1 && c <= UTF16_SURROGATE2) ||
	   (c > RANGE4))
	{
		return ERROR_RUNE_UTF8;
	}

	if(c <= RANGE1){
		res.size = 1;
		res.data[0] = c;
	}
	else if(c <= RANGE2){
		res.size = 2;
		res.data[0] = SIZE2 | ((c >> 6) & MASK2);
		res.data[1] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= RANGE3){
		res.size = 3;
		res.data[0] = SIZE3 | ((c >> 12) & MASK3);
		res.data[1] = CONT  | ((c >> 6) & MASKX);
		res.data[2] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= RANGE4){
		res.size = 4;
		res.data[0] = SIZE4 | ((c >> 18) & MASK4);
		res.data[1] = CONT  | ((c >> 12) & MASKX);
		res.data[2] = CONT  | ((c >> 6)  & MASKX);
		res.data[3] = CONT  | ((c >> 0)  & MASKX);
	}
	return res;
}

constexpr UTF8DecodeResult DECODE_ERROR = { .codepoint = ERROR_RUNE, .size = 0 };

UTF8DecodeResult utf8_decode(Slice<byte> s){
	UTF8DecodeResult res = {};
	byte* buf    = raw_data(s);
	isize buflen = len(s);

	if(buflen <= 0){ return DECODE_ERROR; }

	u8 first = buf[0];

	if((first & CONT) == 0){
		res.size = 1;
		res.codepoint |= first;
	}
	else if ((first & ~MASK2) == SIZE2 && buflen >= 2){
		res.size = 2;
		res.codepoint |= (buf[0] & MASK2) << 6;
		res.codepoint |= (buf[1] & MASKX) << 0;
	}
	else if ((first & ~MASK3) == SIZE3 && buflen >= 3){
		res.size = 3;
		res.codepoint |= (buf[0] & MASK3) << 12;
		res.codepoint |= (buf[1] & MASKX) << 6;
		res.codepoint |= (buf[2] & MASKX) << 0;
	}
	else if ((first & ~MASK4) == SIZE4 && buflen >= 4){
		res.size = 4;
		res.codepoint |= (buf[0] & MASK4) << 18;
		res.codepoint |= (buf[1] & MASKX) << 12;
		res.codepoint |= (buf[2] & MASKX) << 6;
		res.codepoint |= (buf[3] & MASKX) << 0;
	}
	else {
		return DECODE_ERROR;
	}

	// Validation step
	if(res.codepoint >= UTF16_SURROGATE1 && res.codepoint <= UTF16_SURROGATE2){
		return DECODE_ERROR;
	}
	if(res.size > 1 && !is_continuation_byte(buf[1])){
		return DECODE_ERROR;
	}
	if(res.size > 2 && !is_continuation_byte(buf[2])){
		return DECODE_ERROR;
	}
	if(res.size > 3 && !is_continuation_byte(buf[3])){
		return DECODE_ERROR;
	}

	return res;
}

UTF8DecodeResult iter_next_pair(UTF8Iterator* it){
	if(it->current >= len(it->data)){ return {0, 0}; }

	UTF8DecodeResult res = utf8_decode(it->data[ {it->current, len(it->data)} ]);

	if(res.codepoint == DECODE_ERROR.codepoint){
		res.size = 1;
	}

	it->current += res.size;
	return res;
}

UTF8DecodeResult iter_prev_pair(UTF8Iterator* it){
	if(it->current <= 0){ return {0, 0}; }

	it->current -= 1;
	while(is_continuation_byte(it->data[it->current])){
		it->current -= 1;
	}

	UTF8DecodeResult res = utf8_decode(it->data[ {it->current, len(it->data)} ]);

	return res;
}

rune iter_next(UTF8Iterator* it){
	auto res = iter_next_pair(it);
	return res.codepoint;
}

rune iter_prev(UTF8Iterator* it){
	auto res = iter_prev_pair(it);
	return res.codepoint;
}

