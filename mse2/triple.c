// gcc triple.c -o triple -march=haswell -g
#include <immintrin.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#ifndef __AVX__
#error "AVX needed"
#endif

#ifndef __BMI2__
#error 'BMI2 needed"
#endif

const size_t SRC_SIZE = 1ULL << 30; // bytes

uint32_t lut[256];

// Reference implementation, working byte by byte
void basic_stretch_data(void* src, size_t bytes, void* dst) {
    uint8_t* src_c = src;
    uint8_t* dst_c = dst;

    for (int i = 0; i < bytes; ++i) {
	uint32_t t = lut[src_c[i]]; // lut filled below

	dst_c[3*i] = (t & 0xff);
	dst_c[3*i+1] = (t & 0xff00) >> 8;
	dst_c[3*i+2] = (t & 0xff0000) >> 16;
    }
}

// Every three bits toggled, in one of three shifts
#define BIT_MASK_1 0x9249249249249249ULL
#define BIT_MASK_2 (BIT_MASK_1 >> 1)
#define BIT_MASK_3 (BIT_MASK_1 >> 2)

// Expects 32-bit alignment
void pdep_stretch_data(void* src, size_t bytes, void* dst) { 
    uint64_t* src_c = src;
    uint64_t* dst_c = dst;

    uint64_t p1, p2, p3, p4, p5, p6;

    for (int i = 0; i < bytes / 8; i += 4) {
	// Output 96 bytes per loop 
	uint64_t k = src_c[i];

#define GRAB_PARTS(a, b, c)  \
	a = _pdep_u64(k, BIT_MASK_1);	          /* first 22 bits (little endian) */ \
	b = _pdep_u64(k >> 22, BIT_MASK_2);       /* middle 21 bits */ \
	c = _pdep_u64(k >> 43, BIT_MASK_3);       /* last 21 bits */

	GRAB_PARTS(p1, p2, p3)	
	k = src_c[i+1];
	GRAB_PARTS(p4, p5, p6)
	
	_mm256_store_si256((__m256i*) &dst_c[3 * i], _mm256_set_epi64x(p4, p3, p2, p1));

	k = src_c[i+2];
	GRAB_PARTS(p1, p2, p3)

	k = src_c[i+3];
	
	_mm256_store_si256((__m256i*) &dst_c[3 * i + 4], _mm256_set_epi64x(p2, p1, p6, p5));
	GRAB_PARTS(p4, p5, p6)
	    
	_mm256_store_si256((__m256i*) &dst_c[3 * i + 8], _mm256_set_epi64x(p6, p5, p4, p3));
    }
}

#define DEFINE_TRIPLIFY(name, IntType, Out) Out name(IntType b) { \
    Out s = 0; \
    for (int i = 0; i < sizeof(IntType) * CHAR_BIT; ++i) { \
	s |= (b & (1 << i)) << (2 * i); \
    } \
    return s; \
}

DEFINE_TRIPLIFY(triplify8, uint8_t, uint32_t)
DEFINE_TRIPLIFY(triplify16, uint16_t, uint64_t)

void fill_triplify_lut() {
    uint8_t b = 0;
    do {
	lut[b] = triplify8(b);
    } while (++b != 0);
}

// Probably too big to fit in L1d...
uint64_t lut16[65536];

void fill_triplify_lut16() {
    uint16_t b = 0;
    do {
	lut16[b] = triplify8(b);
    } while (++b != 0);
}

clock_t c;
void clock_start() {
    c = clock();
}

void clock_end(const char* msg) {
    printf("Clock %s took %f s\n", msg, (double)(clock() - c) / CLOCKS_PER_SEC);
}

int main() {
    fill_triplify_lut();

    char* src = aligned_alloc(32, SRC_SIZE);
    char* dst = aligned_alloc(32, 3 * SRC_SIZE);

    // Init source with some data
    for (size_t i = 0; i < SRC_SIZE; ++i) {
	src[i] = i;
    }

    // Stretch data so that each bit becomes three bits in the result, with two of them 0
    clock_start(); 
    pdep_stretch_data(src, SRC_SIZE, dst);
    clock_end("pdep finished");
   
    char* dst2 = aligned_alloc(32, 3 * SRC_SIZE);

    clock_start(); 
    basic_stretch_data(src, SRC_SIZE, dst2);
    clock_end("basic finished");

    // Validate
    for (size_t i = 0; i < 3 * SRC_SIZE; ++i) {
	if (dst[i] != dst2[i]) {
	    printf("Results do not match at byte %zu. Expected: %i; got: %i.\n", i, dst2[i], dst[i]);
	    return 0;
	}
    }
}
