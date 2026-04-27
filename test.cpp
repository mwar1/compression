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

TEST_CASE("Testing RLE against Corpus Files") {
    RLE compressor;
    std::string testDir = "./inputs";

    if (!fs::exists(testDir)) {
        MESSAGE("Test directory not found, skipping file tests.");
        return;
    }

    for (const auto& entry : fs::directory_iterator(testDir)) {
        SUBCASE(entry.path().filename().string().c_str()) {
            check(compressor, entry.path().string());
        }
    }
}

TEST_CASE("Edge Cases") {
    RLE compressor;

    SUBCASE("Empty input") {
        std::vector<uint8_t> in = {};
        CHECK(compressor.decode(compressor.encode(in)) == in);
    }

    SUBCASE("Single byte") {
        std::vector<uint8_t> in = { 'A' };
        CHECK(compressor.decode(compressor.encode(in)) == in);
    }

    SUBCASE("Long run (>255 bytes)") {
        std::vector<uint8_t> in(500, 'Z');
        CHECK(compressor.decode(compressor.encode(in)) == in);
    }
}
