#include "../compressor.h"

#include <iostream>
#include <algorithm>
#include <numeric>

// Uses the Larsson-Sadakane algorithm
std::vector<uint32_t> build_suffix_array(const std::vector<uint8_t>& txt) {
    uint32_t n = txt.size();

    std::vector<uint32_t> arr(n);
    std::vector<int32_t> rank(n);
    std::vector<int32_t> temp(n);

    std::iota(arr.begin(), arr.end(), 0);
    for (int i = 0; i < n; i++) rank[i] = txt[i];

    for (int k = 1; k < n; k <<= 1) {
        auto cmp = [&](int i, int j) {
            if (rank[i] != rank[j]) return rank[i] < rank[j];
            int ri = (i + k < n) ? rank[i + k] : -1;
            int rj = (j + k < n) ? rank[j + k] : -1;
            return ri < rj;
        };

        std::sort(arr.begin(), arr.end(), cmp);

        // Compute new ranks
        temp[arr[0]] = 0;
        for (int i = 1; i < n; i++) {
            temp[arr[i]] = temp[arr[i - 1]] + (cmp(arr[i - 1], arr[i]) ? 1 : 0);
        }
        rank = temp;

        // If all ranks are unique, we are done
        if (rank[arr[n - 1]] == n - 1) break;
    }
    return arr;
}

uint32_t get_primary_index(const std::vector<uint32_t>& arr) {
    for (int i = 0; i < arr.size(); ++i) {
        if (arr[i] == 0) {
            return i;
        }
    }
    return -1;
}

/*
    Burrows-Wheeler Transform
*/
class BWT : public Compressor {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;

        int len = txt.size();
        out.reserve(len + sizeof(uint32_t));

        auto suffix_arr = build_suffix_array(txt);
        
        // Insert the primary index
        uint32_t p_idx = get_primary_index(suffix_arr);
        for (int i=0; i<4; i++) {
            out.push_back(static_cast<uint8_t>((p_idx >> (i * 8)) & 0xFF));
        }

        for (int i = 0; i < len; i++) {
            int pos = suffix_arr[i];
            if (pos == 0) {
                out.push_back(txt[len - 1]);
            } else {
                out.push_back(txt[pos - 1]);
            }
        }

        return out;
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;
    
        return txt;
    }
};

static bool registered = []() {
    Compressor::getRegistry()["BWT"] = {
        []() { return std::make_unique<BWT>(); },
        false
    };
    return true;
}();
