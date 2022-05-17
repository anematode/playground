#include <stdint.h>
#include <immintrin.h>

// Credit to Daniel Lemires (lemires.me) for the surrounding code
uint64_t interleave_uint32_with_zeros(uint32_t input) {
	uint64_t word = input;
	word = (word ^ (word << 16)) & 0x0000ffff0000ffff;
	word = (word ^ (word << 8)) & 0x00ff00ff00ff00ff;
	word = (word ^ (word << 4)) & 0x0f0f0f0f0f0f0f0f;
	word = (word ^ (word << 2)) & 0x3333333333333333;
	word = (word ^ (word << 1)) & 0x5555555555555555;
	return word;
}

uint32_t deinterleave_lowuint32(uint64_t word) {
	word &= 0x5555555555555555;
	word = (word ^ (word >> 1)) & 0x3333333333333333;
	word = (word ^ (word >> 2)) & 0x0f0f0f0f0f0f0f0f;
	word = (word ^ (word >> 4)) & 0x00ff00ff00ff00ff;
	word = (word ^ (word >> 8)) & 0x0000ffff0000ffff;
	word = (word ^ (word >> 16)) & 0x00000000ffffffff;
	return (uint32_t)word;
}

typedef struct {
	uint32_t x;
	uint32_t y;
} uint32_2;

uint64_t interleave(uint32_2 input) {
	return interleave_uint32_with_zeros(input.x) |
		(interleave_uint32_with_zeros(input.y) << 1);
}

void interleave_array(uint32_2 *input, size_t length, uint64_t *out) {
	for (size_t i = 0; i < length; i++) {
		out[i] = interleave(input[i]);
	}
}

// I wrote this one
static inline void interleave_clmul(uint32_2 *input, uint64_t *out) {
	__m128i xy = _mm_load_si128((const __m128i *)input);

	xy = _mm_shuffle_epi32(xy, 0b11011000);

	__m128i p2 = _mm_clmulepi64_si128(xy, xy, 0x11);
	__m128i p1 = _mm_clmulepi64_si128(xy, xy, 0x00);

	p2 = _mm_slli_epi16(p2, 1);
	__m128i p3 = _mm_or_si128(p1, p2);

	_mm_storeu_si128((__m128i *)out, p3);
}

void interleave_clmul_array(uint32_2 *input, size_t length, uint64_t *out) {
	size_t i = 0;
	for (; i + 3 < length; i += 4) {
		interleave_clmul(input + i, out + i);
	}

	for (; i < length; i++) {
		out[i] = interleave(input[i]);
	}
}

uint32_2 deinterleave(uint64_t input) {
	uint32_2 answer;
	answer.x = deinterleave_lowuint32(input);
	answer.y = deinterleave_lowuint32(input >> 1);
	return answer;
}

void deinterleave_array(uint64_t *input, size_t length, uint32_2 *out) {
	for (size_t i = 0; i < length; i++) {
		out[i] = deinterleave(input[i]);
	}
}

static inline uint64_t interleave_uint32_with_zeros_pdep(uint32_t input) {
	return _pdep_u64(input, 0x5555555555555555);
}

static inline uint32_t deinterleave_lowuint32_pext(uint64_t word) {
	return (uint32_t)_pext_u64(word, 0x5555555555555555);
}

uint64_t interleave_pdep(uint32_2 input) {
	return _pdep_u64(input.x, 0x5555555555555555) |
		_pdep_u64(input.y, 0xaaaaaaaaaaaaaaaa);
}

void interleave_array_pdep(uint32_2 *input, size_t length, uint64_t *out) {
	for (size_t i = 0; i < length; i++) {
		out[i] = interleave_pdep(input[i]);
	}
}

uint32_2 deinterleave_pext(uint64_t input) {
	uint32_2 answer;
	answer.x = _pext_u64(input, 0x5555555555555555);
	answer.y = _pext_u64(input, 0xaaaaaaaaaaaaaaaa);
	return answer;
}

void deinterleave_array_pext(uint64_t *input, size_t length, uint32_2 *out) {
	for (size_t i = 0; i < length; i++) {
		out[i] = deinterleave_pext(input[i]);
	}
}

int main() {
	// TODO: testing code
}
