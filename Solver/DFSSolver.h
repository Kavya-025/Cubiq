#ifndef CUBIQ_DFSSOLVER_H
#define CUBIQ_DFSSOLVER_H

#include "RubiksCube.h"

#include <vector>

// T is the Rubik's Cube representation and H is its hash type.
// H is kept as a template parameter so the solver remains compatible
// with all three representations and their Task 5 hash types.
template<typename T, typename H>
class DFSSolver {
private:
    T rubiksCube;
    int maxSearchDepth;
    std::vector<RubiksCube::MOVE> solution;

    bool dfs(int depth) {
        if (rubiksCube.isSolved()) {
            return true;
        }

        if (depth >= maxSearchDepth) {
            return false;
        }

        for (int moveIndex = 0; moveIndex < 18; moveIndex++) {
            const auto move =
                static_cast<RubiksCube::MOVE>(moveIndex);

            const T stateBeforeMove = rubiksCube;
            rubiksCube.move(move);
            solution.push_back(move);

            if (dfs(depth + 1)) {
                return true;
            }

            solution.pop_back();
            rubiksCube = stateBeforeMove;
        }

        return false;
    }

public:
    DFSSolver(const T& cube, int maxSearchDepth = 8)
        : rubiksCube(cube),
          maxSearchDepth(maxSearchDepth) {}

    std::vector<RubiksCube::MOVE> solve() {
        solution.clear();

        if (maxSearchDepth < 0) {
            return {};
        }

        if (dfs(0)) {
            return solution;
        }

        solution.clear();
        return solution;
    }

    const T& cube() const {
        return rubiksCube;
    }
};

#endif // CUBIQ_DFSSOLVER_H
