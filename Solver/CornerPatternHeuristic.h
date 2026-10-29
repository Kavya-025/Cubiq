#ifndef CUBIQ_CORNERPATTERNHEURISTIC_H
#define CUBIQ_CORNERPATTERNHEURISTIC_H

#include "../PatternDatabases/CornerPatternDatabase.h"

// Admissible corner-distance heuristic. Unpopulated entries conservatively
// contribute zero; generated partial databases instead store maxDepth + 1.
class CornerPatternHeuristic {
private:
    const CornerPatternDatabase* database_;

public:
    explicit CornerPatternHeuristic(const CornerPatternDatabase& database)
        : database_(&database) {}

    int operator()(const RubiksCube& cube) const {
        const std::uint8_t estimate = database_->getNumMoves(cube);
        return estimate == 0x0F ? 0 : static_cast<int>(estimate);
    }
};

#endif // CUBIQ_CORNERPATTERNHEURISTIC_H
