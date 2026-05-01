#include "../compressor.h"

#include <vector>
#include <numeric>

// Working range
#define RANGE_BITS 32
static_assert(RANGE_BITS <= 32 && RANGE_BITS >= 24, "Working range must be between 24 and 32 bits inclusive.");

// Threshold under which to output a byte
const uint32_t THRESH = (1 << (RANGE_BITS - 8)) ;

void adjust_range(uint8_t c, std::vector<uint32_t>& cum_freq, uint32_t len, uint32_t& low, uint32_t& range) {
    uint64_t slice_low   = (static_cast<uint64_t>(range) *cum_freq[c]) / len;
    uint32_t slice_width = cum_freq[c + 1] - cum_freq[c];

    uint64_t slice_range = (static_cast<uint64_t>(range) * slice_width) / len;
    
    low += static_cast<uint32_t>(slice_low);
    range = static_cast<uint32_t>(slice_range);
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

        std::vector<uint32_t> cum_freq = get_cum_freq(txt);

        uint32_t low = 0;
        uint32_t range = (RANGE_BITS == 32) ? ~0U : (1U << RANGE_BITS) - 1;

        uint8_t cache = 0;
        uint32_t cache_size = 0;
        for (size_t i = 0; i < len; i++) {
            // Inside the for loop
            uint8_t c = txt[i];
            uint32_t old_low = low;

            adjust_range(c, cum_freq, len, low, range);

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
