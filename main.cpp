#include "AppCommands.h"
#include <iostream>
#include <random>
#include <string>
#include <vector>

std::vector<RubiksCube::MOVE> makeRandomScramble(std::size_t moveCount) {
    std::random_device seedSource;
    std::mt19937 randomEngine(seedSource());
    std::uniform_int_distribution<int> moveDistribution(0, 17);

    std::vector<RubiksCube::MOVE> scramble;
    scramble.reserve(moveCount);
    int previousFace = -1;
    while (scramble.size() < moveCount) {
        const int moveIndex = moveDistribution(randomEngine);
        const int face = moveIndex / 3;
        // Avoid adjacent turns of the same face; they combine into one move.
        if (face == previousFace) {
            continue;
        }
        scramble.push_back(static_cast<RubiksCube::MOVE>(moveIndex));
        previousFace = face;
    }
    return scramble;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        return runRandomDepthEightSolve(argv[0]);
    }

    const std::string command(argv[1]);
    if (command == "--test") {
        return runProjectTests();
    }
    if (command == "--random-pq-13") {
        return runSingleRandom13PriorityQueueSolve(argv[0]);
    }
    if (command == "--compare-searches") {
        return runFixedSearchComparison(argv[0]);
    }
    if (command == "--make-corner-db") {
        return makeCornerDatabaseFromArguments(argc, argv);
    }
    if (command == "--solve-with-corner-db") {
        return solveWithCornerDatabaseFromArguments(argc, argv);
    }

    std::cerr << "Usage: cubiq [--test | --random-pq-13 | --compare-searches | "
                 "--make-corner-db ... | --solve-with-corner-db ...]\n";
    return 2;
}
