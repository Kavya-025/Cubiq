#ifndef CUBIQ_BFSSOLVER_H
#define CUBIQ_BFSSOLVER_H

#include "RubiksCube.h"

#include <algorithm>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

// T is a cube representation and H is that representation's hash functor.
template<typename T, typename H>
class BFSSolver {
private:
    T initialCube;

    static RubiksCube::MOVE inverse(RubiksCube::MOVE move) {
        const int moveIndex = static_cast<int>(move);
        const int face = moveIndex / 3;
        const int turn = moveIndex % 3;

        if (turn == 0) {
            return static_cast<RubiksCube::MOVE>(face * 3 + 1);
        }

        if (turn == 1) {
            return static_cast<RubiksCube::MOVE>(face * 3);
        }

        return move;
    }

public:
    explicit BFSSolver(const T& cube)
        : initialCube(cube) {}

    std::vector<RubiksCube::MOVE> solve() const {
        using Move = RubiksCube::MOVE;

        std::queue<T> frontier;
        // Each key is a discovered cube state; its value is the move
        // that reached it from its parent state.
        std::unordered_map<T, Move, H> parentMove;

        parentMove.emplace(initialCube, Move::F); // Root marker.
        frontier.push(initialCube);

        std::optional<T> solvedCube;

        while (!frontier.empty()) {
            T current = frontier.front();
            frontier.pop();

            if (current.isSolved()) {
                solvedCube = current;
                break;
            }

            for (int moveIndex = 0; moveIndex < 18; ++moveIndex) {
                const Move move = static_cast<Move>(moveIndex);
                T next = current;
                next.move(move);

                const auto [entry, inserted] =
                    parentMove.emplace(next, move);

                if (inserted) {
                    frontier.push(next);
                }
            }
        }

        if (!solvedCube) {
            return {};
        }

        // Follow parent links from the solved state back to the input.
        // The recorded moves point forward, so reverse the collected path.
        std::vector<Move> solution;
        T cursor = *solvedCube;

        while (!(cursor == initialCube)) {
            const auto parent = parentMove.find(cursor);
            if (parent == parentMove.end()) {
                return {};
            }

            const Move forwardMove = parent->second;
            solution.push_back(forwardMove);
            cursor.move(inverse(forwardMove));
        }

        std::reverse(solution.begin(), solution.end());
        return solution;
    }
};

#endif // CUBIQ_BFSSOLVER_H
