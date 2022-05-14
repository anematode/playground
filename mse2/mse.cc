// g++ mse.cc -o mse -O3 -march=native -std=c++14

#include <iostream>
#include <immintrin.h>

#ifndef __AVX__
#error "AVX not supported (please define -march=native if your CPU does support it)
#endif

using IType = uint64_t;
const IType SEARCH_MAX = 2'000'000'000ULL;


template<typename IntType=char>
bool extract_bit(IntType i, int idx=0) {
    return !!(i & (1 << idx));
}

// Bitset for reached numbers
char* reached;

void allocate_reached() {
    reached = malloc(SEARCH_MAX / CHAR_BIT);
    if (!reached) {
        throw std::exception("Failed to allocate memory");
    }
}

void fill_reached() {
    // Fill reached with the mapping x -> 2x+1. x -> 3x, x -> 3x+2, x -> 3x+7

    // Fill first 512 bytes
    
}

int main() {
    if (SEARCH_MAX % (256 * CHAR_BIT)) {

    }

    allocate_reached();
}
