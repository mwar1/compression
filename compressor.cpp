#include "compressor.h"

Compressor::Registry& Compressor::getRegistry() {
    static Registry instance;
    return instance;
}
