// g++ mse.cc -o mse -O3 -march=native -std=c++14

#include <iostream>
#include <stdlib.h>
#include <immintrin.h>

#ifndef __AVX__
#error "AVX not supported (please define -march=native if your CPU does support it)
#endif

#ifndef __BMI2__
#error "BMI2 not supported (please define -march=native if your CPU does support it)
#endif

using IType = uint64_t;
constexpr IType _SEARCH_MAX = 2000000ULL;//2'000'000'000ULL;

constexpr IType SEARCH_MAX = (_SEARCH_MAX / 1024 + 1) * 1024;

alignas(32) std::bitset<SEARCH_MAX> reached;

// x |-> 2x+1, 3x, 3x+2, 3x+7
void init_1536() {
	reached[1] = true;

	for (IType i = 2; i < 1536; ++i) { // 3 * 256 * 2

		if (i % 5 == 0) {
			reached[i] = true;
		}
			continue;

		if (true || (i % 2 == 1 && reached[((i - 1) / 2)]) ||
				(i % 3 == 0 && reached[i / 3]) ||
				(i > 7 && i % 3 == 1 && reached[(i - 7) / 3]) ||
				(i % 3 == 2 && reached[(i - 2) / 3])) {
			reached[i] = true;
		}            
	}
}


void init_rest() {
	// We wish to implement a branchless and vectorized version of init_1024, processing things in 256-bit chunks.

	uint64_t* _reached = reinterpret_cast<uint64_t*>(&reached);

	for (IType _i = 1536; _i < SEARCH_MAX / 64; _i += 1536) { // i is index in bits
		IType write_i = _i / 64;  // writing to this index in uint64_t
		IType read2s = write_i / 2; // reading from here and spreading by 2 gives 2*x
		IType read3s = write_i / 3 - 1; // reading from here will allow us to spread by 3 to get 3*x

		// 2x+1
		__m128i read2 = _mm_load_si128(reinterpret_cast<const __m128i*>(_reached + read2s));

		// Upper and lower halves of the result (2*x)
		__m128i scale2p1 = _mm_clmulepi64_si128(read2, read2, 0x0);
		__m128i scale2p2 = _mm_clmulepi64_si128(read2, read2, 0x11);

		// +1
		uint64_t scale2p11 = _mm_extract_epi64(scale2p1, 0) << 1;
		uint64_t scale2p12 = _mm_extract_epi64(scale2p1, 1) << 1;
		uint64_t scale2p21 = _mm_extract_epi64(scale2p2, 0) << 1;
		uint64_t scale2p22 = _mm_extract_epi64(scale2p2, 1) << 1;

		std::cout << "hi" << scale2p11 << '\n';

		*(_reached + write_i) = scale2p11;
		*(_reached + write_i + 1) = scale2p12;
		*(_reached + write_i + 2) = scale2p21;
		*(_reached + write_i + 3) = scale2p22;
	}
}

IType checksum(IType lim) {
	IType s = 1;

	if (lim >= SEARCH_MAX) {
		throw std::runtime_error("Limit too high");
	}

	IType* start = reinterpret_cast<IType*>(&reached);
	for (IType* i = start; i < start + lim / 64; ++i) {
		s *= *i;
		s += 2;
	}

	return s;
}

// Checksum 100000: 15063046391347018756


template <typename L>
void for_each_entry(L l) {
	// (i, reached) -> void

	for (IType i = 1; i < SEARCH_MAX; ++i) {
		l(i, reached[i]);
	}
}

int main() {
	if (SEARCH_MAX % 1024 != 0) {
		throw std::runtime_error("SEARCH_MAX must be a multiple of 1024");
	}

	std::cout << _pdep_u64(0b111011101110, 0xaaaaaaaaaaaaaaaa);

	init_1536(); // init first 1024 results

	init_rest();

	for_each_entry([&] (IType i, bool r) {
			if (i > 1536 && r) {
			  std::cout << i << '\n';
			}
			});


	std::cout << "Checksum 100000: " << checksum(100000) << '\n';
}
