#ifndef CUBIQ_IDDFSSOLVER_H
#define CUBIQ_IDDFSSOLVER_H

#include "RubiksCube.h"

#include <vector>

// T is a cube representation; H is its hash functor, matching the other
// generic solvers in this project.
template<typename T, typename H>
class IDDFSSolver {
private:
    T initialCube;
    int maxSearchDepth;
    std::vector<RubiksCube::MOVE> solution;

    bool depthLimitedSearch(T& cube, int remainingDepth, int previousFace) {
        if (cube.isSolved()) {
            return true;
        }

        if (remainingDepth == 0) {
            return false;
        }

        for (int moveIndex = 0; moveIndex < 18; ++moveIndex) {
            const int face = moveIndex / 3;
            // Consecutive turns of one face can always be combined into a
            // single move (or removed), so they are unnecessary in a shortest path.
            if (face == previousFace) {
                continue;
            }

            const auto move = static_cast<RubiksCube::MOVE>(moveIndex);
            T stateBeforeMove = cube;
            cube.move(move);
            solution.push_back(move);

            if (depthLimitedSearch(cube, remainingDepth - 1, face)) {
                return true;
            }

            solution.pop_back();
            cube = stateBeforeMove;
        }

        return false;
    }

public:
    explicit IDDFSSolver(const T& cube, int maxSearchDepth = 7)
        : initialCube(cube), maxSearchDepth(maxSearchDepth) {}

    std::vector<RubiksCube::MOVE> solve() {
        solution.clear();
        if (maxSearchDepth < 0) {
            return {};
        }

        for (int depthLimit = 0; depthLimit <= maxSearchDepth; ++depthLimit) {
            T current = initialCube;
            solution.clear();
            if (depthLimitedSearch(current, depthLimit, -1)) {
                return solution;
            }
        }

        solution.clear();
        return {};
    }
};

#endif // CUBIQ_IDDFSSOLVER_H
