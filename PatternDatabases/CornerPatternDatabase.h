#ifndef CUBIQ_CORNERPATTERNDATABASE_H
#define CUBIQ_CORNERPATTERNDATABASE_H

#include "PatternDatabase.h"
#include "PermutationIndexer.h"

#include <cstdint>

class CornerPatternDatabase final : public PatternDatabase {
public:
    static constexpr std::uint32_t PERMUTATION_COUNT = 40320; // 8!
    static constexpr std::uint32_t ORIENTATION_COUNT = 2187;  // 3^7
    static constexpr std::uint32_t DATABASE_SIZE =
        PERMUTATION_COUNT * ORIENTATION_COUNT;

    CornerPatternDatabase();
    explicit CornerPatternDatabase(std::uint8_t initialValue);

    std::uint32_t getDatabaseIndex(const RubiksCube& cube) const override;

private:
    PermutationIndexer<8> permutationIndexer_;
};

#endif // CUBIQ_CORNERPATTERNDATABASE_H
