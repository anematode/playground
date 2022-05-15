// g++ mse.cc -o mse -O3 -march=native -std=c++11
#include <iostream>
#include <bitset>

using I = uint64_t;
const I M = 20000000000ULL;
const I SEARCH_SIZE = 200000000000ULL;
I max = 1;
I min = 1;

std::bitset<SEARCH_SIZE> reachable;

// Accepts i>=2
bool is_reachable(I i) {
	// 2*x + 1
	if (i % 2 == 1) {	
		if (reachable[(i-1)/2]) {
			return true;
		}
	}

	switch (i % 3) {
		// 3*x
		case 0:
			if (reachable[i / 3]) {
				return true;
			}
			break;
		// 3*x + 2
		case 2:
			if (reachable[(i - 2) / 3]) {
				return true;
			}
			break;
		// 3*x + 7
		case 1:

			if (i >= 7 && reachable[(i - 7) / 3]) {
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
        if ((negate != reachable[i]) && l(i)) {
            if (limit != -1 && cnt++ > limit) {
                std::cout << "Output limit exceeded\n";
                break;
            }

            std::cout << i << '\n';
        }

    }
}

int main() {
    reachable[1] = 1;

    const int PROGRESS_EVERY = SEARCH_SIZE / 100;

    // Initialize
    for (I i = 2; i < SEARCH_SIZE; ++i) {
        if (is_reachable(i)) {
		reachable[i] = 1;
	}
        if (i % PROGRESS_EVERY == 0) {
		std::cout << "Progress: " << i << '/' << SEARCH_SIZE << '\n';
	}
    }

    std::cout << "Congruent to 3 mod 4\n";
    print_satisfying([=] (I i) {
        return i % 4 == 3;
    }, true);
    std::cout << "Congruent to 7 mod 8\n";
    print_satisfying([=] (I i) {
        return i % 8 == 7;
    }, true);


    /*for (I i = 0; i < M; ++i) {
        if (reachable[i]) {
            if (!reachable[3 * i + 2]) {
                std::cout << i << " is not closed!\n";
                return 0;
            }
        }
    }*/
}
