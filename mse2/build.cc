// Input file format:
// UNREACHS<64 bit int> ...
// g++ build.cc -o build -std=c++17 -O3 -march=native -g


// We only store the unreachable positions, and use them to construct the next unreachable positions.
// For compactness we hash the positions as follows: (upper 48 bits) -> (array of lower 16 bits).

#include <fstream>
#include <vector>
#include <iostream>
#include <exception>
#include <cstring>
#include <thread>
#include <array>
#include <cstdint>

// The end of computation is assumed to be equal to the last entry in the file. The file format begins with
// an 8-byte header "UNREACHS" and then 64-bit integers
const std::string UNREACHABLE_FILE {"./unreachable2trillion.dat"};

const bool USE_THREADS = true;
const int NUM_THREADS = 8;

std::vector<uint64_t> read_unreachable_data() {
	std::ifstream in(UNREACHABLE_FILE, std::ios::binary | std::ios::in);
	in.unsetf(std::ios::skipws);

	if (!in.is_open()) {
		throw std::runtime_error("Unable to open file " + UNREACHABLE_FILE);
	}

	in.seekg(0, std::ios::end);
	std::streampos file_size = in.tellg();
	in.seekg(0, std::ios::beg);

	const char* MAGIC_HEADER = "UNREACHS";

	char header[9] = { '\0' };
	header[8] = '\0';

	in.read(header, std::strlen(MAGIC_HEADER));

	if (std::strcmp(MAGIC_HEADER, header)) {
		throw std::runtime_error("Unknown header " + std::string(header));
	}

	std::vector<uint64_t> v;
	v.resize(file_size / 8 - 1);

	in.read(reinterpret_cast<std::ifstream::char_type*>(&v.front()), file_size);

	return v;
}

using vec64iter = std::vector<uint64_t>::iterator;

std::vector<vec64iter> split_work(vec64iter from, vec64iter to, int threads) {
	std::ptrdiff_t cnt = to - from;

	std::vector<vec64iter> v;

	for (int i = 0; i <= threads; ++i) {
		v.push_back(from + i * cnt / threads);
	}

	return v;
}

template <int M>
void count_congruences(vec64iter from,
		vec64iter to,
		std::array<uint64_t, M>& results,
		std::mutex& results_mutex) {
	std::array<uint64_t, M> unreachable = {0};

	for (; from < to; ++from) {
		unreachable[*from % M]++;
	}

	std::lock_guard<std::mutex> guard { results_mutex };
	for (int i = 0; i < M; ++i) {
		results[i] += unreachable[i];
	}
}

enum class Format {
	Numerical,
	Pastable
};

template <int M, Format f=Format::Numerical>
void print_moduli(std::vector<uint64_t> v) {
	auto chunks = split_work(v.begin(), v.end(), NUM_THREADS);	
	std::array<uint64_t, M> results = {0};

	std::mutex results_mutex;

	std::vector<std::thread> threads;
	for (int i = 0; i < NUM_THREADS; ++i) {
		std::thread th ([&] (int i) {
			count_congruences<M>(chunks[i], chunks[i + 1], results, results_mutex);
		}, i);
		threads.push_back(std::move(th));
	}

	for (auto& th : threads) {
		th.join();
	}

	std::cout << "Congruences mod " << M << ":\n";
	for (int i = 0; i < M; ++i)  {
		if constexpr (f == Format::Numerical) {
			std::cout << "=" << i << ": " << results[i] << " (" << (double)results[i] / v.size() * 100 << "%)\n";
		} else {
			std::cout << i << '\t' << results[i] << '\n';
		}
	}
}

template <typename L>
void print_satisfying(std::vector<uint64_t> v, L l) {
	for (auto i : v) {
		if (l(i)) {
			std::cout << i << '\n';
		}
	}
}

int main() {
	auto v = read_unreachable_data();	

	std::cout << "Read " << v.size() << " unreachable integers." << '\n';

	uint64_t cnt = 0;
	for (int i = 0; i < v.size(); ++i) {
		if (v[i] % 8 == 7) {
			std::cout << i << '\t' << v[i] << '\n';
		}
	}

	print_moduli<13, Format::Pastable>(v);
}
