#include "../compressor.h"

#include <vector>
#include <numeric>

// Working range
#define RANGE_BITS 32
static_assert(RANGE_BITS <= 32 && RANGE_BITS >= 24, "Working range must be between 24 and 32 bits inclusive.");

// How often to adjust cumulative frequencies
#define RESCALE_LIM 0x4000

// Threshold under which to output a byte
const uint32_t THRESH = (1 << (RANGE_BITS - 8));

int lsb(int n) {
    return n & (-n);
}

// Turns a vector of frequencies into a Fenwick tree
void build_fenwick(std::vector<uint32_t>& values) {  
    for (int idx = 1; idx < values.size(); idx++) {
        int parentIdx = idx + lsb(idx);
        if (parentIdx <= values.size()) {
            values[parentIdx] += values[idx];
        }
    }
}

// Get the cumulative frequency up to the
// character with index <idx>
int query_fenwick(std::vector<uint32_t>& tree, int idx) {
    int sum = 0;
    while (idx > 0) {
        sum += tree[idx];
        idx -= lsb(idx);
    }

    return sum;
}

// Add one to the character with index <idx>
void update_fenwick(std::vector<uint32_t>& tree, int idx) {
    idx++;
    while (idx < tree.size()) {
        tree[idx]++;
        idx += lsb(idx);
    }
}

// Adjust the Fenwick tree to make cumulative frequencies half what
// they originally were.
// Ensures very infrequent characters don't end up rounding to 0
void rescale_fenwick(std::vector<uint32_t>& tree) {
    std::vector<uint32_t> counts(257);

    for (int i = 0; i < 256; i++) {
        uint32_t c = query_fenwick(tree, i + 1) - query_fenwick(tree, i);
        // Halve the counts, but ensure it's always at least 1
        counts[i + 1] = (c >> 1) | 1;
    }
    
    // Rebuild using the new counts
    tree = std::move(counts);
    build_fenwick(tree);
}

void adjust_range(uint8_t c, std::vector<uint32_t>& cum_freq, uint32_t& low, uint32_t& range) {
    uint32_t total_freq = query_fenwick(cum_freq, 256);

    uint32_t freq_low  = query_fenwick(cum_freq, c);
    uint32_t freq_high = query_fenwick(cum_freq, c + 1);
    uint32_t slice_width = freq_high - freq_low;
    
    uint64_t scaled_low   = (static_cast<uint64_t>(range) * freq_low) / total_freq;
    uint64_t scaled_range = (static_cast<uint64_t>(range) * slice_width) / total_freq;
    
    low += static_cast<uint32_t>(scaled_low);
    range = static_cast<uint32_t>(scaled_range);
    
    update_fenwick(cum_freq, c);
}

void emit_byte(uint32_t byte, std::vector<uint8_t>& out, uint8_t& cache, uint32_t& cache_size) {
    if (byte < 0xFF) {
        out.push_back(cache);
        for (; cache_size > 0; cache_size--) out.push_back(0xFF);
        cache = byte;
    } else if (byte == 0xFF) {
        cache_size++;
    } else {
        out.push_back(cache + 1);
        for (; cache_size > 0; cache_size--) out.push_back(0x00);
        cache = static_cast<uint8_t>(byte); 
    }
}

class Range_Enc : public Compressor {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;

        const uint32_t len = txt.size();
        write_size(out, len);

        // Assume uniform distribution of characters initially
        std::vector<uint32_t> cum_freq(257, 1);
        cum_freq[0] = 0;

        build_fenwick(cum_freq);

        uint32_t low = 0;
        uint32_t range = (RANGE_BITS == 32) ? ~0U : (1U << RANGE_BITS) - 1;

        uint8_t cache = 0;
        uint32_t cache_size = 0;
        for (size_t i = 0; i < len; i++) {
            // Inside the for loop
            uint8_t c = txt[i];
            uint32_t old_low = low;

            adjust_range(c, cum_freq, low, range);

            if (query_fenwick(cum_freq, 256) > RESCALE_LIM) {
                rescale_fenwick(cum_freq);
            }

            // Check for overflow
            if (low < old_low) {
                emit_byte(0x100, out, cache, cache_size);
            }

            while (range < THRESH) {
                emit_byte(low >> (RANGE_BITS - 8), out, cache, cache_size);

                low   <<= 8;
                range <<= 8;
            }
        }

        // Output remaining bytes
        for (int i = 0; i < 4; ++i) {
            uint8_t b = static_cast<uint8_t>(low >> 24);
            
            emit_byte(b, out, cache, cache_size);
            low <<= 8;
        }
    
        return out;
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
    
        return txt;
    }
};

static bool registered = []() {
    Compressor::getRegistry()["Range"] = []() { return std::make_unique<Range_Enc>(); };
    return true;
}();
