#include "compressor.h"
#include "version.h"

#include <iostream>
#include <fstream>
#include <vector>
#include "argparse/argparse.hpp"

void display_algos() {
    std::cout << "Available compression algorithms:" << std::endl;
    auto& registry = Compressor::getRegistry();
    if (registry.empty()) {
        std::cout << "No algorithms found." << std::endl;
    } else {
        for (auto const& [name, entry] : registry) {
            if (entry.visible) {
                std::cout << "- " << name << std::endl;
            }
        }
    }
}

int transform(const std::string& algo, const std::string& input_path, const std::string& output_path, bool compress) {
    auto& registry = Compressor::getRegistry();
    auto it = registry.find(algo);

    if (it == registry.end() || !it->second.visible) {
        std::cerr << "Algorithm " << algo << " not found." << std::endl;
        return 1;
    }

    std::ifstream in_file(input_path, std::ios::binary);
    if (!in_file) {
        std::cerr << "Could not open input file: " << input_path << std::endl;
        return 1;
    }

    std::vector<uint8_t> input_bytes((std::istreambuf_iterator<char>(in_file)), std::istreambuf_iterator<char>());
    in_file.close();

    auto c = registry.at(algo).creator();
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

    std::cout << (compress ? "Compression" : "Decompression") << " successful: " << output_path << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("compressor", PROJECT_VERSION);

    // Subcommands
    argparse::ArgumentParser list_command("list");
    list_command.add_description("List all available compression algorithms");

    argparse::ArgumentParser compress_command("compress");
    compress_command.add_argument("algo").help("Algorithm to use");
    compress_command.add_argument("input").help("Input file");
    compress_command.add_argument("output").help("Output file");

    argparse::ArgumentParser decompress_command("decompress");
    decompress_command.add_argument("algo").help("Algorithm to use");
    decompress_command.add_argument("input").help("Input file");
    decompress_command.add_argument("output").help("Output file");

    program.add_subparser(list_command);
    program.add_subparser(compress_command);
    program.add_subparser(decompress_command);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        // If there's an argument after the program name, it's likely the subcommand
        if (argc > 1) {
            std::string sub = argv[1];
            if (sub == "compress") {
                std::cerr << compress_command << std::endl;
            } else if (sub == "decompress") {
                std::cerr << decompress_command << std::endl;
            } else if (sub == "list") {
                std::cerr << list_command << std::endl;
            } else {
                std::cerr << program << std::endl;
            }
        } else {
            std::cerr << program << std::endl;
        }
        return 1;
    }

    // Check which subcommand was used and execute
    if (program.is_subcommand_used(list_command)) {
        display_algos();
    } else if (program.is_subcommand_used(compress_command)) {
        return transform(
            compress_command.get<std::string>("algo"),
            compress_command.get<std::string>("input"),
            compress_command.get<std::string>("output"),
            true
        );
    } else if (program.is_subcommand_used(decompress_command)) {
        return transform(
            decompress_command.get<std::string>("algo"),
            decompress_command.get<std::string>("input"),
            decompress_command.get<std::string>("output"),
            false
        );
    } else {
        // No subcommand provided
        std::cerr << program << std::endl;
        return 1;
    }

    return 0;
}
