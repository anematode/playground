#include <bitset>
#include <iostream>

int main() {
    std::bitset<32 * 10> b;

    for (int i = 0; i < 100; ++i) {
	if (i % 3 == 0) {
	    b[i] = true;
	}
    }

    for (int i = 0; i < 32 * 10 / 64; ++i) {
	std::cout << int(*(reinterpret_cast<uint64_t*>(&b) + i)) << " \n";
    }

    
}
