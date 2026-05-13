#include "../compressor.h"

#include <numeric>

/*
    Move-to-front transform
*/
class MTF : public Compressor {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return {};

        std::vector<uint8_t> alphabet(256, 0);
        std::iota(alphabet.begin(), alphabet.end(), 0);

        for (uint8_t b : txt) {
            // Find the index of the character in the alphabet
            uint8_t idx = 0;
            for (int i = 0; i < 256; ++i) {
                if (alphabet[i] == b) {
                    idx = static_cast<uint8_t>(i);
                    break;
                }
            }

            out.push_back(idx);
            
            // Move this character to the front of the alphabet
            for (int i = idx; i > 0; i--) {
                alphabet[i] = alphabet[i - 1];
            }
            alphabet[0] = b;
        }
    
        return out;
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;

        std::vector<uint8_t> alphabet(256, 0);
        std::iota(alphabet.begin(), alphabet.end(), 0);

        for (uint8_t b : txt) {
            uint8_t character = alphabet[b];
            out.push_back(character);

            for (int i = b; i > 0; i--) {
                alphabet[i] = alphabet[i - 1];
            }
            alphabet[0] = character;
        }
    
        return out;
    }
};

static bool registered = []() {
    Compressor::getRegistry()["MTF"] = {
        []() { return std::make_unique<MTF>(); },
        false
    };
    return true;
}();
