#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <vector>
#include <cstdint>
#include <map>
#include <string>
#include <functional>
#include <memory>

class Compressor {
public:
    virtual ~Compressor() = default;

    virtual std::vector<uint8_t> encode(const std::vector<uint8_t>& data) const = 0;
    virtual std::vector<uint8_t> decode(const std::vector<uint8_t>& data) const = 0;

    using Registry = std::map<std::string, std::function<std::unique_ptr<Compressor>()>>;
    static Registry& getRegistry();
};

#endif
