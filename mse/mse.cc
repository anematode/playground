// g++ mse.cc -o mse -O3 -march=native -std=c++11
#include <iostream>
#include <immintrin.h>

using I = uint64_t;
const I M = 20000000000ULL;
const I SEARCH_SIZE = 200000000000ULL;

I max = 1;
I min = 1;

// ptr to SEARCH_SIZE / 8 bytes of data
char* reached;

// x -> 2x + 1, x -> 3x, x -> 3x + 2, x -> 3x + 7

bool get_bit(I b) {
	I a = reached[b / CHAR_BIT];

	return a & (1 << (b % CHAR_BIT));
}

// Accepts i>=2
bool is_reached(I i) {
	// 2*x + 1
	if (i % 2 == 1) {	
		if (reached[(i-1)/2]) {
			return true;
		}
	}

	switch (i % 3) {
		// 3*x
		case 0:
			if (reached[i / 3]) {
				return true;
			}
			break;
			// 3*x + 2
		case 2:
			if (reached[(i - 2) / 3]) {
				return true;
			}
			break;
			// 3*x + 7
		case 1:

			if (i >= 7 && reached[(i - 7) / 3]) {
				return true;
			}
			break;
	}

	return false;
}

template <typename L>
void print_satisfying(L l, bool negate=false, int limit=100) {
	std::ios_base::sync_with_stdio(false);

	int cnt = 0;
	for (I i = 0; i < SEARCH_SIZE; ++i) {
		if ((negate != reached[i]) && l(i)) {
			if (limit != -1 && cnt++ > limit) {
				std::cout << "Output limit exceeded\n";
				break;
			}

			std::cout << i << '\n';
		}

	}
}

int main() {
	reached[1] = 1;

	const int PROGRESS_EVERY = SEARCH_SIZE / 100;

	// Initialize
	for (I i = 2; i < SEARCH_SIZE; ++i) {
		if (is_reached(i)) {
			reached[i] = 1;
		}
		if (i % PROGRESS_EVERY == 0) {
			std::cout << "Progress: " << i << '/' << SEARCH_SIZE << '\n';
		}
	}

	for (int m = 2; m <= 5; ++m) {

		std::cout << "Congruent to 3 mod 4\n";
		print_satisfying([=] (I i) {
				return i % 4 == 3;
				}, true);
		std::cout << "Congruent to 7 mod 8\n";
		print_satisfying([=] (I i) {
				return i % 8 == 7;
				}, true);
	}
}

