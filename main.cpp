#include "compressor.h"

#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Invalid usage:\n./compressor <algo> <input> <output>" << std::endl;
        return 1;
    }

    std::string algo = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    std::ifstream in_file(input, std::ios::binary);
    std::vector<uint8_t> input_bytes = {std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>()};
    in_file.close();

    bool match = false;
    auto& registry = Compressor::getRegistry();
    for (auto const& [name, creator] : registry) {
        if (name == algo) {
            match = true;

            auto c = creator();
            std::vector<uint8_t> compressed = c->encode(input_bytes);
            
            std::ofstream outFile(output, std::ios::binary);

            if (!outFile) {
                std::cerr << "Could not open file for writing: " + output << std::endl;
                return 1;
            }

            outFile.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
            outFile.close();

            break;
        }
    }

    if (!match) {
        std::cerr << "Algorithm " << algo << " not found." << std::endl;
        return 1;
    }

    return 0;
}