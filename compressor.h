#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <functional>
#include <memory>

class Compressor;

struct AlgoEntry {
    std::function<std::unique_ptr<Compressor>()> creator;
    bool visible = true;
};

class Compressor {
public:
    virtual ~Compressor() = default;

    virtual std::vector<uint8_t> encode(const std::vector<uint8_t>& data) const = 0;
    virtual std::vector<uint8_t> decode(const std::vector<uint8_t>& data) const = 0;

    using Registry = std::map<std::string, AlgoEntry>;
    static Registry& getRegistry();
};

void write_size(std::vector<uint8_t>& f, uint64_t sz);
std::vector<int> get_freqs(const std::vector<uint8_t>& txt);
std::vector<uint32_t> get_cum_freq(const std::vector<uint8_t>& txt);

#endif
