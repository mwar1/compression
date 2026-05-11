#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "compressor.h"
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::vector<uint8_t> load_input(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void check(const Compressor& comp, const std::string& filePath) {
    std::vector<uint8_t> original = load_input(filePath);

    auto encoded = comp.encode(original);
    auto decoded = comp.decode(encoded);

    CHECK(original == decoded);
}

TEST_CASE("Testing against Corpus Files") {
    std::string testDir = "../inputs";
    if (!fs::exists(testDir)) {
        MESSAGE("Test directory not found, skipping file tests.");
        return;
    }

    static std::vector<fs::path> testFiles;
    if (testFiles.empty() && fs::exists(testDir)) {
        for (const auto& entry : fs::recursive_directory_iterator(testDir)) {
            if (fs::is_regular_file(entry)) testFiles.push_back(entry.path());
        }
    }
    
    auto& registry = Compressor::getRegistry();
    for (auto const& [name, creator] : registry) {
        SUBCASE(name.c_str()) {
            auto c = creator();

            for (const auto& path : testFiles) {
                SUBCASE(path.filename().string().c_str()) {
                    check(*c, path.string());
                }
            }
        }
    }
}

TEST_CASE("Edge Cases") {
    auto& registry = Compressor::getRegistry();
    for (auto const& [name, creator] : registry) {
        SUBCASE(name.c_str()) {
            auto c = creator();

            SUBCASE("Empty input") {
                std::vector<uint8_t> in = {};
                CHECK(c->decode(c->encode(in)) == in);
            }

            SUBCASE("Single byte") {
                std::vector<uint8_t> in = { 'A' };
                CHECK(c->decode(c->encode(in)) == in);
            }

            SUBCASE("Long run (>255 bytes)") {
                std::vector<uint8_t> in(500, 'Z');
                CHECK(c->decode(c->encode(in)) == in);
            }
        }
    }
}
