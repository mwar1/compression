#include "../compressor.h"

class RLE : public Compressor {
public:
    std::vector<uint8_t> encode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
        if (txt.empty()) return out;
    
        int rep = 1;
        uint8_t last = txt.at(0);
        for (size_t i=1; i<txt.size(); i++) {
            if (txt.at(i) == last && rep < 255) {
                rep++;
            } else {
                out.push_back(static_cast<uint8_t>(rep));
                out.push_back(last);
                last = txt.at(i);
                rep = 1;
            }
        }
    
        out.push_back(static_cast<uint8_t>(rep));
        out.push_back(last);
    
        return out;
    }
    
    std::vector<uint8_t> decode(const std::vector<uint8_t>& txt) const override {
        std::vector<uint8_t> out;
    
        for (size_t i=0; i+1<txt.size(); i+=2) {
            uint8_t count = txt.at(i);
            uint8_t val = txt.at(i + 1);
            
            for (int j = 0; j < count; j++) {
                out.push_back(val);
            }
        }
    
        return out;
    }
};

static bool registered = []() {
    Compressor::getRegistry()["RLE"] = []() { return std::make_unique<RLE>(); };
    return true;
}();
