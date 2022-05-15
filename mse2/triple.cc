#include <immintrin.h>
#include <time.h>
#include <stdio.h>

#ifndef __AVX__
#error "AVX needed"
#endif

#ifndef __BMI2__
#error 'BMI2 needed"
#endif

const size_t SRC_SIZE = 1 << 20; // bytes

uint32_t triplify(uint8_t b) {
    uint32_t s = 0;
    for (int i = 0; i < 8; ++i) {
	s |= (b & (1 << i)) << (3 * i);
    }
    return s;
}

void reference_impl_stretch_data(void* src, size_t bytes, void* dst) {
    uint32_t lut[256]; // map byte
}

clock_t c;
void clock_start() {
    c = clock();
}

void clock_end(const char* msg="unnamed") {
    printf("Clock %s took %f ms", msg, (double)(clock() - c) / CLOCKS_PER_SEC);
}

int main() {
    char* src = malloc(SRC_SIZE / 8);
    char* dst = malloc(3 * SRC_SIZE / 8);

    printf("%i", triplify(0b111101));

    // Init source with some data
    for (size_t i = 0; i < SRC_SIZE / 8; ++i) {
	src[i] = i;
    }

    // Stretch data so that each bit becomes three bits in the result, with two of them 0
    clock_start(); 
    stretch_data(src, SRC_SIZE, dst);
    clock_end("stretch data finished");
   
    char* dst2 = malloc(3 * SRC_SIZE / 8);

    clock_start(); 
    reference_impl_stretch_data(src, SRC_SIZE, dst);
    clock_end("reference stretch data finished");

    // Validate
    for (size_t i = 0; i < 3 * SRC_SIZE / 8; ++i) {
	if (dst[i] != dst2[i]) {
	    printf("Results do not match at byte %zu", i);
	}
    }
}
