#ifndef CUBIQ_PERMUTATION_INDEXER_H
#define CUBIQ_PERMUTATION_INDEXER_H

#include "math.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

// Lehmer-code ranker for permutations containing each value in [0, N) once.
template<std::size_t N>
class PermutationIndexer {
    static_assert(N > 0 && N <= 256,
                  "PermutationIndexer requires 1 <= N <= 256");

private:
    std::array<std::uint32_t, N> factorialWeights_{};

public:
    PermutationIndexer() {
        for (std::size_t position = 0; position < N; ++position) {
            factorialWeights_[position] = factorial(
                static_cast<std::uint32_t>(N - position - 1));
        }
    }

    std::uint32_t rank(const std::array<std::uint8_t, N>& permutation) const {
        std::array<bool, N> seen{};
        std::uint64_t result = 0;

        for (std::size_t position = 0; position < N; ++position) {
            const std::size_t value = permutation[position];
            if (value >= N || seen[value]) {
                throw std::invalid_argument(
                    "PermutationIndexer input must be a permutation of [0, N)");
            }

            std::uint32_t smallerUnused = 0;
            for (std::size_t candidate = 0; candidate < value; ++candidate) {
                if (!seen[candidate]) {
                    ++smallerUnused;
                }
            }

            result += static_cast<std::uint64_t>(smallerUnused) *
                      factorialWeights_[position];
            seen[value] = true;
        }
        return static_cast<std::uint32_t>(result);
    }
};

#endif // CUBIQ_PERMUTATION_INDEXER_H
