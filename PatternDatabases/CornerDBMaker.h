#ifndef CUBIQ_CORNERDBMAKER_H
#define CUBIQ_CORNERDBMAKER_H

#include "CornerPatternDatabase.h"

#include <cstddef>
#include <cstdint>

class CornerDBMaker {
public:
    struct Result {
        std::size_t discoveredStates;
        std::size_t expandedStates;
        std::size_t peakFrontier;
    };

    // Refuses to exceed this frontier size so deeper database generation
    // cannot consume unbounded memory on the user's machine.
    static constexpr std::size_t DEFAULT_MAX_FRONTIER = 500000;

    explicit CornerDBMaker(
        std::size_t maxFrontier = DEFAULT_MAX_FRONTIER);

    Result generate(CornerPatternDatabase& database,
                    std::uint8_t maxDepth) const;

private:
    std::size_t maxFrontier_;
};

#endif // CUBIQ_CORNERDBMAKER_H
