#ifndef CUBIQ_IDASTARSOLVER_H
#define CUBIQ_IDASTARSOLVER_H

#include "RubiksCube.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

struct IDAstarOptions {
    std::uint64_t maxNodes = 5'000'000;
    std::size_t maxOpenStates = 1'000'000;
    std::chrono::seconds maxDuration{30};
    std::uint64_t progressIntervalNodes = 250'000;
    std::function<void(int, std::uint64_t, std::chrono::milliseconds)> onProgress;
};

enum class IDAstarStatus {
    NotRun,
    Searching,
    Solved,
    NodeLimitReached,
    FrontierLimitReached,
    TimeLimitReached,
    Exhausted
};

// Placeholder heuristic for the first IDA* implementation. Returning zero
// keeps the heuristic admissible, but gives no guidance to the search.
template<typename T>
class ZeroHeuristic {
public:
    int operator()(const T&) const noexcept {
        return 0;
    }
};

// T is a cube representation, H its hash functor (kept consistent with the
// project's other generic solvers), and Heuristic estimates distance to goal.
template<typename T, typename H, typename Heuristic = ZeroHeuristic<T>>
class IDAstarSolver {
private:
    T initialCube;
    Heuristic heuristic;
    IDAstarOptions options;
    std::vector<RubiksCube::MOVE> solution;
    IDAstarStatus status = IDAstarStatus::NotRun;
    std::uint64_t visitedNodes = 0;
    std::uint64_t nextProgressNode = 0;
    int currentBound = 0;
    std::chrono::steady_clock::time_point startedAt;

    static constexpr int FOUND = -1;
    static constexpr int ABORTED = -2;

    bool shouldStop() {
        if (visitedNodes >= options.maxNodes) {
            status = IDAstarStatus::NodeLimitReached;
            return true;
        }

        // Check the clock periodically so the hot recursive loop stays cheap.
        if ((visitedNodes & 0xFFFu) == 0 &&
            std::chrono::steady_clock::now() - startedAt >= options.maxDuration) {
            status = IDAstarStatus::TimeLimitReached;
            return true;
        }
        return false;
    }

    void reportProgress() {
        if (!options.onProgress || options.progressIntervalNodes == 0 ||
            visitedNodes < nextProgressNode) {
            return;
        }

        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startedAt);
        options.onProgress(currentBound, visitedNodes, elapsed);
        nextProgressNode = visitedNodes + options.progressIntervalNodes;
    }

    int search(T& cube, int depth, int bound, int previousFace) {
        if (shouldStop()) {
            return ABORTED;
        }
        ++visitedNodes;
        reportProgress();

        const int estimate = heuristic(cube);
        const int score = depth + estimate;
        if (score > bound) {
            return score;
        }

        if (cube.isSolved()) {
            return FOUND;
        }

        int nextBound = std::numeric_limits<int>::max();
        for (int moveIndex = 0; moveIndex < 18; ++moveIndex) {
            const int face = moveIndex / 3;
            // Consecutive turns on the same face can be combined into one
            // move, so they are not needed in a shortest solution.
            if (face == previousFace) {
                continue;
            }

            const auto move = static_cast<RubiksCube::MOVE>(moveIndex);
            T child = cube;
            child.move(move);
            solution.push_back(move);

            const int result = search(child, depth + 1, bound, face);
            if (result == FOUND) {
                return FOUND;
            }
            if (result == ABORTED) {
                return ABORTED;
            }

            solution.pop_back();
            nextBound = std::min(nextBound, result);
        }

        return nextBound;
    }

public:
    explicit IDAstarSolver(const T& cube,
                           Heuristic heuristic = Heuristic{},
                           IDAstarOptions options = {})
        : initialCube(cube), heuristic(std::move(heuristic)),
          options(std::move(options)) {}

    std::vector<RubiksCube::MOVE> solve() {
        solution.clear();
        visitedNodes = 0;
        currentBound = heuristic(initialCube);
        startedAt = std::chrono::steady_clock::now();
        nextProgressNode = options.progressIntervalNodes;
        status = IDAstarStatus::Searching;
        int bound = heuristic(initialCube);

        while (true) {
            currentBound = bound;
            if (shouldStop()) {
                solution.clear();
                return {};
            }
            T current = initialCube;
            const int nextBound = search(current, 0, bound, -1);
            if (nextBound == FOUND) {
                status = IDAstarStatus::Solved;
                return solution;
            }
            if (nextBound == ABORTED) {
                solution.clear();
                return {};
            }
            if (nextBound == std::numeric_limits<int>::max()) {
                status = IDAstarStatus::Exhausted;
                solution.clear();
                return {};
            }

            solution.clear();
            bound = nextBound;
        }
    }

    IDAstarStatus getStatus() const noexcept { return status; }
    std::uint64_t getVisitedNodes() const noexcept { return visitedNodes; }
    int getCurrentBound() const noexcept { return currentBound; }
};

#endif // CUBIQ_IDASTARSOLVER_H
