// g++ mse.cc -o mse -O3 -march=native -std=c++14

#include <iostream>
#include <bitset>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>

#ifndef __AVX__
#error "AVX not supported (please define -march=native if your CPU does support it)"
#endif

#ifndef __BMI2__
#error "BMI2 not supported (please define -march=native if your CPU does support it)"
#endif

using IType = uint64_t;
constexpr IType _SEARCH_MAX = 2'000'000'000ULL;

constexpr IType SEARCH_MAX = (_SEARCH_MAX / 1024 + 1) * 1024;

alignas(32) std::bitset<SEARCH_MAX> reached;
alignas(32) std::bitset<SEARCH_MAX> reached2;

// x |-> 2x+1, 3x, 3x+2, 3x+7
void init_1536(IType cnt=1536) {
	reached[1] = true;


	for (IType i = 2; i <= cnt; ++i) { // 3 * 256 * 2
		if ((i % 2 == 1 && reached[((i - 1) / 2)]) ||
				(i % 3 == 0 && reached[i / 3]) ||
				(i > 7 && i % 3 == 1 && reached[(i - 7) / 3]) ||
				(i % 3 == 2 && reached[(i - 2) / 3])) {
			reached[i] = true;
		}            
	}
}


// Every three bits toggled, in one of three locations
#define BIT_MASK_1 0x9249249249249249ULL
#define BIT_MASK_2 (BIT_MASK_1 >> 1)
#define BIT_MASK_3 (BIT_MASK_1 >> 2)

void init_rest() {
	// We wish to implement a branchless and vectorized version of init_1024, processing things in 256-bit chunks.

	const uint64_t start = 1536;
	uint64_t* _reached = reinterpret_cast<uint64_t*>(&reached);

	__m128i read2, scale2p1, scale2p2;
	uint64_t scale2p12, scale2p21, scale2p11, scale2p22 = 0;
	uint64_t scale3p03, scale3p10, scale3p11, scale3p12 = _pdep_u64(_reached[start / 3 - 1], BIT_MASK_3), scale2p02;
	uint64_t store1, store2, store3, store4, store5, store6;

	for (IType _i = start; _i < SEARCH_MAX; _i += 768 /* 64 * 12 */) { // i is index in bits
		IType write_i = _i / 64;  // writing to this index in uint64_t
		IType read2s = write_i / 2; // reading from here and spreading by 2 gives 2*x
		IType read3s = write_i / 3 - 1; // reading from here will allow us to spread by 3 to get 3*x

#define compute_scale2_parts(addr) \
		/* 2x+1 */ \
		read2 = _mm_load_si128(reinterpret_cast<const __m128i*>(_reached + addr)); \
		/* Upper and lower halves of the result (2*x). throughput 1, latency 6, easily hidden */ \
		scale2p1 = _mm_clmulepi64_si128(read2, read2, 0x0); \
		scale2p2 = _mm_clmulepi64_si128(read2, read2, 0x11);

#define extract_scale2 \
		scale2p02 = scale2p22; \
		scale2p11 = _mm_extract_epi64(scale2p1, 0); \
		scale2p12 = _mm_extract_epi64(scale2p1, 1); \
		scale2p21 = _mm_extract_epi64(scale2p2, 0); \
		scale2p22 = _mm_extract_epi64(scale2p2, 1); \

		compute_scale2_parts(read2s);

		// We wish to extract 3x, 3x+2, and 3x+7. We do so with pdeps and shifts, first extracting 3x,
		// then performing some shifts

		// 3x+a
		uint64_t scale3p1 = _reached[read3s + 1];
		uint64_t scale3p2 = _reached[read3s + 2];
		uint64_t scale3p3 = _reached[read3s + 3];
		uint64_t scale3p4 = _reached[read3s + 4];

#define extract_scale3(src) \
		scale3p03 = scale3p12; \
		scale3p10 = _pdep_u64(src, BIT_MASK_1); \
		scale3p11 = _pdep_u64(src >> 22, BIT_MASK_2); \
		scale3p12 = _pdep_u64(src >> 43, BIT_MASK_3); 

#define compute_scale3_parts(store1, store2, store3) \
		store1 = (scale3p03 >> 57) | (scale3p10 << 2) | scale3p10 | (scale3p10 << 7) | (scale3p03 >> 62); \
		store2 = ((scale3p10 >> 57) | (scale3p11 << 2) | scale3p11 | (scale3p11 << 7)) | (scale3p10 >> 62); \
		store3 = ((scale3p11 >> 57) | (scale3p12 << 2) | scale3p12 | (scale3p12 << 7)) | (scale3p11 >> 62); 	

		extract_scale3(scale3p1);

		// Attempt to hide pdep latency
		extract_scale2;

		compute_scale3_parts(store1, store2, store3);

		// Not dependent on previous
		extract_scale3(scale3p2);

		compute_scale3_parts(store4, store5, store6);

#define merge_parts(store1, store2, store3, store4) \
		store1 |= (scale2p11 << 1) | (scale2p02 >> 63); \
		store2 |= (scale2p12 << 1) | (scale2p11 >> 63); \
		store3 |= (scale2p21 << 1) | (scale2p12 >> 63); \
		store4 |= (scale2p22 << 1) | (scale2p21 >> 63);

#define write_data(offset, store1, store2, store3, store4) \
		/* Write accumulated data */ \
		*(_reached + write_i + offset) = store1; \
		*(_reached + write_i + offset + 1) = store2; \
		*(_reached + write_i + offset + 2) = store3; \
		*(_reached + write_i + offset + 3) = store4;

		merge_parts(store1, store2, store3, store4);

		write_data(0, store1, store2, store3, store4);

		compute_scale2_parts(read2s + 2);

		extract_scale3(scale3p3);

		extract_scale2;

		compute_scale3_parts(store1, store2, store3);

		merge_parts(store5, store6, store1, store2);

		write_data(4, store5, store6, store1, store2);

		extract_scale3(scale3p4);

		compute_scale2_parts(read2s + 4);

		compute_scale3_parts(store4, store5, store6);

		extract_scale2;

		merge_parts(store3, store4, store5, store6);

		write_data(8, store3, store4, store5, store6);
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

clock_t c;
void clock_start() {
	c = clock();
}

void clock_end(const char* msg) {
	printf("Clock %s took %f s\n", msg, (double)(clock() - c) / CLOCKS_PER_SEC);
}

int main() {
	if (SEARCH_MAX % 1024 != 0) {
		throw std::runtime_error("SEARCH_MAX must be a multiple of 1024");
	}

	// Force page allocation
	memset((void*)&reached, 0, SEARCH_MAX / 8);

	clock_start();
	memset((void*)&reached, 0, SEARCH_MAX / 8);
	init_1536(); // init first results
	clock_end("standard method");

	clock_start();
	init_rest();
	clock_end("enjoyer method");

	for_each_entry([&] (IType i, bool r) {
			if (!r && i % 4 == 3) {
			std::cout << i << '\n';
			}
			});

	std::cout << "Checksum 100000: " << checksum(100000) << '\n';
}
