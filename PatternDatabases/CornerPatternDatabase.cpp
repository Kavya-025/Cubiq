#include "CornerPatternDatabase.h"

#include <array>
#include <stdexcept>

CornerPatternDatabase::CornerPatternDatabase()
    : PatternDatabase(DATABASE_SIZE) {}

CornerPatternDatabase::CornerPatternDatabase(std::uint8_t initialValue)
    : PatternDatabase(DATABASE_SIZE, initialValue) {}

std::uint32_t CornerPatternDatabase::getDatabaseIndex(
    const RubiksCube& cube
) const {
    std::array<std::uint8_t, 8> cornerPermutation{};
    for (std::uint8_t position = 0; position < cornerPermutation.size(); ++position) {
        cornerPermutation[position] = cube.getCornerIndex(position);
    }

    const std::uint32_t permutationRank =
        permutationIndexer_.rank(cornerPermutation);

    // The eighth corner's orientation is determined by the other seven.
    std::uint32_t orientationRank = 0;
    for (std::uint8_t position = 0; position < 7; ++position) {
        const std::uint8_t orientation = cube.getCornerOrientation(position);
        if (orientation > 2) {
            throw std::invalid_argument("Corner orientation must be 0, 1, or 2");
        }
        orientationRank = orientationRank * 3 + orientation;
    }

    return permutationRank * ORIENTATION_COUNT + orientationRank;
}
