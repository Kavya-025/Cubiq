#ifndef CUBIQ_APP_COMMANDS_H
#define CUBIQ_APP_COMMANDS_H
#include "RubiksCube.h"
#include <cstddef>
#include <string>
#include <vector>
int runProjectTests();
std::string moveName(RubiksCube::MOVE move);
std::vector<RubiksCube::MOVE> makeRandomScramble(std::size_t moveCount);
int runRandomDepthEightSolve(const char* executablePath);
int runSingleRandom13PriorityQueueSolve(const char* executablePath);
int runFixedSearchComparison(const char* executablePath);
int makeCornerDatabaseFromArguments(int argc, char* argv[]);
int solveWithCornerDatabaseFromArguments(int argc, char* argv[]);
#endif
