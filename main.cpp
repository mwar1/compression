#include "compressor.h"

#include <iostream>
#include <fstream>
#include "argparse/argparse.hpp"

void display_algos() {
    std::cout << "Available compression algorithms:" << std::endl;
    auto& registry = Compressor::getRegistry();
    if (registry.empty()) {
        std::cout << "No algorithms found." << std::endl;
    } else {
        for (auto const& [name, creator] : registry) {
            std::cout << "- " << name << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("compressor");

    program.add_argument("--list")
        .help("List all available compression algorithms")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("algo")
        .help("Compression algorithm to use")
        .default_value(std::string(""));
    program.add_argument("input")
        .help("Input file path")
        .default_value(std::string(""));
    program.add_argument("output")
        .help("Output file path")
        .default_value(std::string(""));

    program.add_argument("-d", "--decompress")
        .help("Run in decompression mode")
        .default_value(false)
        .implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program.get<bool>("--list")) {
        display_algos();
        return 0;
    }

    auto algo = program.get<std::string>("algo");
    auto input_path = program.get<std::string>("input");
    auto output_path = program.get<std::string>("output");
    bool compress = !program.get<bool>("--decompress");

    if (algo.empty() || input_path.empty() || output_path.empty()) {
        std::cerr << "Error: Invalid arguments\n";
        std::cerr << program;
        return 1;
    }

    std::ifstream in_file(input_path, std::ios::binary);
    if (!in_file) {
        std::cerr << "Could not open input file: " << input_path << std::endl;
        return 1;
    }

    std::vector<uint8_t> input_bytes = {std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>()};
    in_file.close();

    bool match = false;
    auto& registry = Compressor::getRegistry();
    for (auto const& [name, creator] : registry) {
        if (name == algo) {
            match = true;

            auto c = creator();
            std::vector<uint8_t> output;
            if (compress) {
                output = c->encode(input_bytes);
            } else {
                output = c->decode(input_bytes);
            }
            
            std::ofstream outFile(output_path, std::ios::binary);

            if (!outFile) {
                std::cerr << "Could not open file for writing: " + output_path << std::endl;
                return 1;
            }

            outFile.write(reinterpret_cast<const char*>(output.data()), output.size());
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