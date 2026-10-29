#include "CornerDBMaker.h"

#include "RubiksCubeBitboard.h"

#include <queue>
#include <stdexcept>
#include <utility>

namespace {
struct SearchNode {
    RubiksCubeBitboard cube;
    std::uint8_t depth;
};
}

CornerDBMaker::CornerDBMaker(std::size_t maxFrontier)
    : maxFrontier_(maxFrontier) {
    if (maxFrontier_ == 0) {
        throw std::invalid_argument("Corner DB frontier limit must be positive");
    }
}

CornerDBMaker::Result CornerDBMaker::generate(
    CornerPatternDatabase& database,
    std::uint8_t maxDepth
) const {
    if (database.getSize() != CornerPatternDatabase::DATABASE_SIZE) {
        throw std::invalid_argument("Corner database has an unexpected size");
    }
    if (maxDepth > 14) {
        throw std::invalid_argument(
            "Maximum depth must be 14 or less to fit the nibble values");
    }

    // Every unvisited corner state is at least maxDepth + 1 moves away.
    // This keeps the partial database admissible for IDA*.
    const auto unvisitedEstimate = static_cast<std::uint8_t>(maxDepth + 1);
    database.reset(unvisitedEstimate);

    std::queue<SearchNode> frontier;
    RubiksCubeBitboard solved;
    const std::uint32_t solvedIndex = database.getDatabaseIndex(solved);
    database.setNumMoves(solvedIndex, 0);
    frontier.push(SearchNode{solved, 0});

    Result result{1, 0, 1};
    while (!frontier.empty()) {
        SearchNode current = std::move(frontier.front());
        frontier.pop();
        ++result.expandedStates;

        if (current.depth >= maxDepth) {
            continue;
        }

        const auto childDepth = static_cast<std::uint8_t>(current.depth + 1);
        for (int moveIndex = 0; moveIndex < 18; ++moveIndex) {
            RubiksCubeBitboard child = current.cube;
            child.move(static_cast<RubiksCube::MOVE>(moveIndex));
            const std::uint32_t index = database.getDatabaseIndex(child);

            // BFS encounters each abstract corner state first at its
            // shortest distance. Already-known shallower/equal values skip it.
            if (database.getNumMoves(index) <= childDepth) {
                continue;
            }

            if (frontier.size() >= maxFrontier_) {
                throw std::runtime_error(
                    "Corner DB generation reached its frontier safety limit; "
                    "try a smaller max depth or raise the explicit limit");
            }

            database.setNumMoves(index, childDepth);
            frontier.push(SearchNode{std::move(child), childDepth});
            ++result.discoveredStates;
            if (frontier.size() > result.peakFrontier) {
                result.peakFrontier = frontier.size();
            }
        }
    }

    return result;
}
