// Input file format:
// UNREACHS<64 bit int> ...


// We only store the unreachable positions, and use them to construct the next unreachable positions.
// For compactness we hash the positions as follows: (upper 48 bits) -> (array of lower 16 bits).

#include <fstream>
#include <vector>
#include <iostream>
#include <exception>
#include <cstring>

// The end of computation is assumed to be equal to the last entry in the file. The file format begins with
// an 8-byte header "UNREACHS" and then 64-bit integers
const std::string UNREACHABLE_FILE {"./unreachable.dat"};

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

int main() {
	auto v = read_unreachable_data();

	std::cout << v.size() << '\n';

	for (uint64_t i : v) {
		if (i % 4 == 3) {
			std::cout << i << '\n';
		}
	}
}
