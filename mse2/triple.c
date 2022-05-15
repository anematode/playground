// gcc triple.c -o triple -march=haswell -g
#include <immintrin.h>
#include <time.h>
#include <stdio.h>

#ifndef __AVX__
#error "AVX needed"
#endif

#ifndef __BMI2__
#error 'BMI2 needed"
#endif

const size_t SRC_SIZE = 1ULL << 5; // bytes

// Every three bits toggled, in one of three shifts
#define BIT_MASK_1 0x9249249249249249ULL
#define BIT_MASK_2 (BIT_MASK_1 >> 1)
#define BIT_MASK_3 (BIT_MASK_1 >> 2)
#define	popcnt __builtin_popcountll

void stretch_data(void* src, size_t bytes, void* dst) {
    uint64_t* src_c = src; // assume proper alignment
    uint64_t* dst_c = dst;

    for (int i = 0; i < bytes / 8; ++i) {
	// Process 64 bytes at a time?
	uint64_t k = src_c[i];
	//printf("%llu\n", k);
	//printf("%llu\n", k & ((1 << 22) - 1));

#define trim_bits(k, count) (k & ~((1ULL << count) - 1))

	dst_c[3*i] = _pdep_u64(k, BIT_MASK_1); // first 22 bits (little endian)
	dst_c[3*i+1] = _pdep_u64(k >> 22, BIT_MASK_2);    // middle 21 bits
	dst_c[3*i+2] = _pdep_u64(k >> 43, BIT_MASK_3);    // last 21 bits
    }
}

uint32_t triplify(uint8_t b) {
    uint32_t s = 0;
    for (int i = 0; i < 8; ++i) {
	s |= (b & (1 << i)) << (2 * i);
    }
    return s;
}

void reference_impl_stretch_data(void* src, size_t bytes, void* dst) {
    uint32_t lut[256];
    uint8_t b = 0;
    do {
	lut[b] = triplify(b);
    } while (++b != 0);

    uint8_t* src_c = src;
    uint8_t* dst_c = dst;

    for (int i = 0; i < bytes; ++i) {
	uint32_t t = lut[src_c[i]];

	dst_c[3*i] = (t & 0xff);
	dst_c[3*i+1] = (t & 0xff00) >> 8;
	dst_c[3*i+2] = (t & 0xff0000) >> 16;
    }
}

clock_t c;
void clock_start() {
    c = clock();
}

void clock_end(const char* msg) {
    printf("Clock %s took %f ms\n", msg, (double)(clock() - c) / CLOCKS_PER_SEC);
}

int main() {
    char* src = malloc(SRC_SIZE);
    char* dst = malloc(3 * SRC_SIZE);

    // Init source with some data
    for (size_t i = 0; i < SRC_SIZE; ++i) {
	src[i] = i;
    }

    // Stretch data so that each bit becomes three bits in the result, with two of them 0
    clock_start(); 
    stretch_data(src, SRC_SIZE, dst);
    clock_end("stretch data finished");
   
    char* dst2 = malloc(3 * SRC_SIZE);

    clock_start(); 
    reference_impl_stretch_data(src, SRC_SIZE, dst2);
    clock_end("reference stretch data finished");

    // Validate
    for (size_t i = 0; i < 3 * SRC_SIZE; ++i) {
	printf("%i ", dst[i]);
	if (0 && dst[i] != dst2[i]) {
	    printf("Results do not match at byte %zu. Expected: %i; got: %i.\n", i, dst2[i], dst[i]);
	    return 0;
	}
    }
    printf("\n");
    for (size_t i = 0; i < 3 * SRC_SIZE; ++i) {
	printf("%i ", dst2[i]);
	if (0 && dst[i] != dst2[i]) {
	    printf("Results do not match at byte %zu. Expected: %i; got: %i.\n", i, dst2[i], dst[i]);
	    return 0;
	}
    }
}
