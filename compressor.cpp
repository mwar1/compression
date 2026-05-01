#include "compressor.h"

Compressor::Registry& Compressor::getRegistry() {
    static Registry instance;
    return instance;
}

// Write the original size of the input, little-endian
void write_size(std::vector<uint8_t>& f, uint64_t sz) {
    for (int i=0; i<8; i++) {
        f.push_back(static_cast<uint8_t>((sz >> (i * 8)) & 0xFF));
    }
}

// Get the frequency of every byte in the input <txt>
std::vector<int> get_freqs(const std::vector<uint8_t>& txt) {
    std::vector<int> counts(256, 0);

    for (uint8_t c : txt) {
        counts[c]++;
    }

    return counts;
}

// cum_freq[i] is the total frequencies of the characters before character i
// cum_freq[i+1] is the total frequencies of characters up to and including character i
std::vector<uint32_t> get_cum_freq(const std::vector<uint8_t>& txt) {
    std::vector<uint32_t> counts(256, 0);

    for (uint8_t c : txt) {
        counts[c]++;
    }

    std::vector<uint32_t> cum_freq(257, 0);
    for (uint32_t i = 0; i < 256; i++) {
        cum_freq[i + 1] = cum_freq[i] + counts[i];
    }

    return cum_freq;
}
