// g++ mse.cc -o mse -O3 -march=native -std=c++14

#include <iostream>
#include <stdlib.h>
#include <immintrin.h>

#ifndef __AVX__
#error "AVX not supported (please define -march=native if your CPU does support it)
#endif

using IType = uint64_t;
constexpr IType _SEARCH_MAX = 2000000ULL;//2'000'000'000ULL;

constexpr IType SEARCH_MAX = (_SEARCH_MAX / 1024 + 1) * 1024;

alignas(32) std::bitset<SEARCH_MAX> reached;

// x |-> 2x+1, 3x, 3x+2, 3x+7
void init_1536() {
    reached[1] = true;

    for (IType i = 2; i < 1536; ++i) { // 3 * 256 * 2
        if ((i % 2 == 1 && reached[((i - 1) / 2)]) ||
                (i % 3 == 0 && reached[i / 3]) ||
                (i > 7 && i % 3 == 1 && reached[(i - 7) / 3]) ||
                (i % 3 == 2 && reached[(i - 2) / 3])) {
            reached[i] = true;
        }            
    }
}

void init_rest() {
    // We wish to implement a branchless and vectorized version of init_1024, processing things in 256-bit chunks. Alas,
    // this is not trivial. We essentially generate four "streams" of 256-bit chunks (one for each of the mappings)
    // constructed from memory, and unroll vehemently.
    
    char* _reached = reinterpret_cast<char*>(&reached);

    for (IType i = 6; i < SEARCH_MAX / 256; i += 6) { // in 256-bit chunk units
        void* write_addr = _reached + (i * 32);

        char* flr2_addr = _reached + (i * 32) / 2;
        char* flr3_addr = _reached + ((i / 3 - 1) * 32);

        __m256 flr2_p1 = _mm256_load_si256((__m256i*) flr2_addr);
        __m256 flr2_p2 = _mm256_load_si256((__m256i*) (flr2_addr + 32));

        __m256 b = _mm256_setzero_si256();

        uint64_t p = _pdep_u64(0, 0);

        __m256 flr3_p1 = _mm256_load_si256((__m256i*) flr3_addr);



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

    init_1536(); // init first 1024 results

    init_rest();

    for_each_entry([&] (IType i, bool r) {
            if (i < 10000 && !r && i % 4 == 3) {
            std::cout << i << '\n';
            }
            });


    std::cout << "Checksum 100000: " << checksum(100000) << '\n';
}
