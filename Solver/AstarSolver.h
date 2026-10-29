#ifndef CUBIQ_ASTARSOLVER_H
#define CUBIQ_ASTARSOLVER_H

#include "IDAstarSolver.h"

#include <queue>
#include <unordered_map>

// Priority-queue, iterative-bound search modeled on the reference solver.
// It uses a boolean visited map and has explicit node, time, and frontier
// limits to keep runs bounded.
template<typename T, typename H, typename Heuristic = ZeroHeuristic<T>>
class AstarSolver {
private:
    struct Node {
        T cube;
        int depth;
        int estimate;
        RubiksCube::MOVE moveFromParent = RubiksCube::MOVE::U;
        int previousFace = -1;
    };

    struct StateInfo {
        bool visited = false;
        RubiksCube::MOVE moveFromParent = RubiksCube::MOVE::U;
    };

    struct LowerPriority {
        bool operator()(const Node& left, const Node& right) const {
            const int leftScore = left.depth + left.estimate;
            const int rightScore = right.depth + right.estimate;
            if (leftScore != rightScore) {
                return leftScore > rightScore;
            }
            return left.estimate > right.estimate;
        }
    };

    static RubiksCube::MOVE inverseMove(RubiksCube::MOVE move) {
        switch (move) {
            case RubiksCube::MOVE::F: return RubiksCube::MOVE::FPRIME;
            case RubiksCube::MOVE::FPRIME: return RubiksCube::MOVE::F;
            case RubiksCube::MOVE::F2: return RubiksCube::MOVE::F2;
            case RubiksCube::MOVE::U: return RubiksCube::MOVE::UPRIME;
            case RubiksCube::MOVE::UPRIME: return RubiksCube::MOVE::U;
            case RubiksCube::MOVE::U2: return RubiksCube::MOVE::U2;
            case RubiksCube::MOVE::L: return RubiksCube::MOVE::LPRIME;
            case RubiksCube::MOVE::LPRIME: return RubiksCube::MOVE::L;
            case RubiksCube::MOVE::L2: return RubiksCube::MOVE::L2;
            case RubiksCube::MOVE::R: return RubiksCube::MOVE::RPRIME;
            case RubiksCube::MOVE::RPRIME: return RubiksCube::MOVE::R;
            case RubiksCube::MOVE::R2: return RubiksCube::MOVE::R2;
            case RubiksCube::MOVE::D: return RubiksCube::MOVE::DPRIME;
            case RubiksCube::MOVE::DPRIME: return RubiksCube::MOVE::D;
            case RubiksCube::MOVE::D2: return RubiksCube::MOVE::D2;
            case RubiksCube::MOVE::B: return RubiksCube::MOVE::BPRIME;
            case RubiksCube::MOVE::BPRIME: return RubiksCube::MOVE::B;
            case RubiksCube::MOVE::B2: return RubiksCube::MOVE::B2;
        }
        return move;
    }

    T initialCube;
    Heuristic heuristic;
    IDAstarOptions options;
    IDAstarStatus status = IDAstarStatus::NotRun;
    std::uint64_t visitedNodes = 0;
    std::uint64_t nextProgressNode = 0;
    std::size_t peakFrontier = 0;
    int currentBound = 0;
    std::chrono::steady_clock::time_point startedAt;

    bool shouldStop() {
        if (visitedNodes >= options.maxNodes) {
            status = IDAstarStatus::NodeLimitReached;
            return true;
        }
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

    std::vector<RubiksCube::MOVE> reconstruct(
        const T& solved,
        const std::unordered_map<T, StateInfo, H>& states
    ) const {
        std::vector<RubiksCube::MOVE> path;
        T current = solved;
        while (!(current == initialCube)) {
            const auto state = states.find(current);
            if (state == states.end()) {
                return {};
            }
            path.push_back(state->second.moveFromParent);
            current.move(inverseMove(state->second.moveFromParent));
        }
        std::reverse(path.begin(), path.end());
        return path;
    }

public:
    explicit AstarSolver(const T& cube,
                         Heuristic heuristic = Heuristic{},
                         IDAstarOptions options = {})
        : initialCube(cube), heuristic(std::move(heuristic)),
          options(std::move(options)) {}

    std::vector<RubiksCube::MOVE> solve() {
        visitedNodes = 0;
        peakFrontier = 0;
        currentBound = heuristic(initialCube);
        startedAt = std::chrono::steady_clock::now();
        nextProgressNode = options.progressIntervalNodes;
        status = IDAstarStatus::Searching;

        int bound = currentBound;
        while (true) {
            currentBound = bound;
            std::priority_queue<Node, std::vector<Node>, LowerPriority> frontier;
            std::unordered_map<T, StateInfo, H> states;

            frontier.push(Node{initialCube, 0, heuristic(initialCube),
                               RubiksCube::MOVE::U, -1});
            peakFrontier = std::max(peakFrontier, frontier.size());
            int nextBound = std::numeric_limits<int>::max();

            while (!frontier.empty()) {
                if (shouldStop()) {
                    return {};
                }

                Node current = frontier.top();
                frontier.pop();
                auto state = states.try_emplace(
                    current.cube, StateInfo{false, current.moveFromParent}).first;
                if (state->second.visited) {
                    continue;
                }
                state->second.visited = true;

                ++visitedNodes;
                reportProgress();
                if (current.cube.isSolved()) {
                    status = IDAstarStatus::Solved;
                    return reconstruct(current.cube, states);
                }

                for (int moveIndex = 0; moveIndex < 18; ++moveIndex) {
                    const int face = moveIndex / 3;
                    if (face == current.previousFace) {
                        continue;
                    }
                    const auto move = static_cast<RubiksCube::MOVE>(moveIndex);
                    T child = current.cube;
                    child.move(move);
                    const int childDepth = current.depth + 1;
                    const int estimate = heuristic(child);
                    const int score = childDepth + estimate;

                    if (score > bound) {
                        nextBound = std::min(nextBound, score);
                        continue;
                    }

                    const auto known = states.find(child);
                    if (known != states.end() && known->second.visited) {
                        continue;
                    }

                    if (frontier.size() >= options.maxOpenStates) {
                        status = IDAstarStatus::FrontierLimitReached;
                        return {};
                    }

                    frontier.push(Node{std::move(child), childDepth, estimate,
                                       move, face});
                    peakFrontier = std::max(peakFrontier, frontier.size());
                }
            }

            if (nextBound == std::numeric_limits<int>::max()) {
                status = IDAstarStatus::Exhausted;
                return {};
            }
            bound = nextBound;
        }
    }

    IDAstarStatus getStatus() const noexcept { return status; }
    std::uint64_t getVisitedNodes() const noexcept { return visitedNodes; }
    std::size_t getPeakFrontier() const noexcept { return peakFrontier; }
    int getCurrentBound() const noexcept { return currentBound; }
};

#endif // CUBIQ_ASTARSOLVER_H
