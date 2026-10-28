#include "DataProcessor.h"
#include <limits>

uint32_t DataProcessor::calculateSumOfSquares(const std::vector<uint32_t>& v) const {
    constexpr uint64_t kMax = std::numeric_limits<uint32_t>::max();
    uint64_t sum = 0;
    for (uint32_t x : v) {
        uint64_t sq = static_cast<uint64_t>(x) * x;
        if (sq > kMax - sum) return static_cast<uint32_t>(kMax);
        sum += sq;
    }
    return static_cast<uint32_t>(sum);
}
