#include "math.h"

#include <limits>
#include <stdexcept>

std::uint32_t factorial(std::uint32_t n) {
    std::uint64_t result = 1;
    for (std::uint32_t factor = 2; factor <= n; ++factor) {
        result *= factor;
        if (result > std::numeric_limits<std::uint32_t>::max()) {
            throw std::overflow_error("factorial does not fit in uint32_t");
        }
    }
    return static_cast<std::uint32_t>(result);
}

std::uint32_t pick(std::uint32_t n, std::uint32_t k) {
    if (k > n) {
        return 0;
    }
    if (k == 0) {
        return 1;
    }

    std::uint64_t result = 1;
    for (std::uint32_t factor = n - k + 1; factor <= n; ++factor) {
        result *= factor;
        if (result > std::numeric_limits<std::uint32_t>::max()) {
            throw std::overflow_error("permutation count does not fit in uint32_t");
        }
        if (factor == n) {
            break; // Avoid unsigned overflow when n is UINT32_MAX.
        }
    }
    return static_cast<std::uint32_t>(result);
}

std::uint32_t choose(std::uint32_t n, std::uint32_t k) {
    if (k > n) {
        return 0;
    }
    k = (k < n - k) ? k : n - k;

    std::uint64_t result = 1;
    for (std::uint32_t i = 1; i <= k; ++i) {
        result = result * (n - k + i) / i;
        if (result > std::numeric_limits<std::uint32_t>::max()) {
            throw std::overflow_error("combination count does not fit in uint32_t");
        }
    }
    return static_cast<std::uint32_t>(result);
}
