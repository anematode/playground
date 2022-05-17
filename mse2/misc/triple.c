// gcc triple.c -o triple -march=haswell -g -O2
#include <immintrin.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#ifndef __AVX2__
#error "AVX2 needed"
#endif

#ifndef __BMI2__
#error "BMI2 needed"
#endif

const size_t SRC_SIZE = 1ULL << 20; // bytes

uint32_t lut[256];
// Probably too big to fit in L1d...
uint64_t lut16[65536];

// Reference implementation, working byte by byte (~7.200 cycles / byte)
void basic_stretch_data(void* src, size_t bytes, void* dst) {
    uint8_t* src_c = src;
    uint8_t* dst_c = dst;

    for (size_t i = 0; i < bytes; ++i) {
	uint32_t t = lut[src_c[i]]; // lut filled below

	dst_c[3*i] = (t & 0xff);
	dst_c[3*i+1] = (t & 0xff00) >> 8;
	dst_c[3*i+2] = (t & 0xff0000) >> 16;
    }
}

// Attempts to be a bit more clever, working in 16-bit chunks and writing in 64-bit chunks (~5.01 cycles / byte).
// Expects 8-byte alignment.
void improved_stretch_data(void* src, size_t bytes, void* dst) {
    uint16_t* src_c = src;
    uint64_t* dst_c = dst;

    for (size_t i = 0; i < bytes / 2; i += 4) {
	// 48 bytes per cycle
	uint64_t t1 = lut16[src_c[i]]; // lut filled below
	uint64_t t2 = lut16[src_c[i+1]];
	uint64_t t3 = lut16[src_c[i+2]];
	uint64_t t4 = lut16[src_c[i+3]];

	dst_c[3*i/4] = t1 | ((t2 & 0xffff) << 48);
	dst_c[3*i/4+1] = ((t2 & 0xffffffff0000) >> 16) | ((t3 & 0xffffffff) << 32);
	dst_c[3*i/4+2] = ((t3 & 0xffff00000000) >> 32) | (t4 << 16);
    }
}

// Every three bits toggled, in one of three locations
#define BIT_MASK_1 0x9249249249249249ULL
#define BIT_MASK_2 (BIT_MASK_1 >> 1)
#define BIT_MASK_3 (BIT_MASK_1 >> 2)

// Expects 32-byte alignment. ~3.02 cycles / byte
void pdep_stretch_data(void* src, size_t bytes, void* dst) { 
    uint64_t* src_c = src;
    uint64_t* dst_c = dst;

    uint64_t k;

    for (size_t i = 0; i < bytes / 8; ) {
	// Output 96 bytes per loop 
	uint64_t k = src_c[i];

#define WRITE_PARTS  \
	k = src_c[i]; \
	dst_c[3*i] = _pdep_u64(k, BIT_MASK_1);	          /* first 22 bits (little endian) */ \
	dst_c[3*i+1] = _pdep_u64(k >> 22, BIT_MASK_2);       /* middle 21 bits */ \
	dst_c[3*i+2] = _pdep_u64(k >> 43, BIT_MASK_3);       /* last 21 bits */ \
	i += 1;

	WRITE_PARTS
	WRITE_PARTS	
	WRITE_PARTS
	WRITE_PARTS
    }
}

#define DEFINE_TRIPLIFY(name, IntType, Out) Out name(IntType b) { \
    Out s = 0; \
    for (int i = 0; i < sizeof(IntType) * CHAR_BIT; ++i) { \
	s |= (Out)(b & (1 << i)) << (2 * i); \
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

void fill_triplify_lut16() {
    uint16_t b = 0;

    do {
	lut16[b] = triplify16(b); 
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
    fill_triplify_lut16();

    char* src = aligned_alloc(32, SRC_SIZE);
    char* dst = aligned_alloc(32, 3 * SRC_SIZE);
    char* dst2 = aligned_alloc(32, 3 * SRC_SIZE);

    printf("Initializing data\n");

    for (size_t i = 0; i < SRC_SIZE; ++i) {
	src[i] = i;
    }

    memset(dst, 3 * SRC_SIZE, 1);
    memset(dst2, 3 * SRC_SIZE, 1);

    printf("Initialized data\n");

    // Stretch data so that each bit becomes three bits in the result, with two of them 0
    clock_start(); 
    for (int i = 0; i < 1000; ++i) {
    pdep_stretch_data(src, SRC_SIZE, dst);
    }
    clock_end("pdep finished"); 

    clock_start(); 
    for (int i = 0; i < 1000; ++i) {
    basic_stretch_data(src, SRC_SIZE, dst2);
    }
    clock_end("basic finished");

    // Validate
    for (size_t i = 0; i < 3 * SRC_SIZE; ++i) {
	if (dst[i] != dst2[i]) {
	    printf("Results do not match at byte %zu. Expected: %i; got: %i.\n", i, dst2[i], dst[i]);
	    return 0;
	}
    }
}
