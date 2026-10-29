#include "RubiksCube1DArray.h"
#include "RubiksCube3DArray.h"
#include "RubiksCubeBitboard.h"
#include "Solver/IDAstarSolver.h"
#include "Solver/AstarSolver.h"
#include "Solver/CornerPatternHeuristic.h"
#include "PatternDatabases/NibbleArray.h"
#include "PatternDatabases/PatternDatabase.h"
#include "PatternDatabases/CornerPatternDatabase.h"
#include "PatternDatabases/CornerDBMaker.h"
#include "PatternDatabases/PermutationIndexer.h"
#include "AppCommands.h"
#include <array>
#include <chrono>
#include <charconv>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
std::string moveName(RubiksCube::MOVE move) {
    switch (move) {
        case RubiksCube::MOVE::F:      return "F";
        case RubiksCube::MOVE::FPRIME: return "F'";
        case RubiksCube::MOVE::F2:     return "F2";
        case RubiksCube::MOVE::U:      return "U";
        case RubiksCube::MOVE::UPRIME: return "U'";
        case RubiksCube::MOVE::U2:     return "U2";
        case RubiksCube::MOVE::L:      return "L";
        case RubiksCube::MOVE::LPRIME: return "L'";
        case RubiksCube::MOVE::L2:     return "L2";
        case RubiksCube::MOVE::R:      return "R";
        case RubiksCube::MOVE::RPRIME: return "R'";
        case RubiksCube::MOVE::R2:     return "R2";
        case RubiksCube::MOVE::D:      return "D";
        case RubiksCube::MOVE::DPRIME: return "D'";
        case RubiksCube::MOVE::D2:     return "D2";
        case RubiksCube::MOVE::B:      return "B";
        case RubiksCube::MOVE::BPRIME: return "B'";
        case RubiksCube::MOVE::B2:     return "B2";
    }

    return "?";
}

bool parseMoveToken(const std::string& token, RubiksCube::MOVE& move) {
    static const std::pair<const char*, RubiksCube::MOVE> moves[] = {
        {"F", RubiksCube::MOVE::F}, {"F'", RubiksCube::MOVE::FPRIME},
        {"F2", RubiksCube::MOVE::F2}, {"U", RubiksCube::MOVE::U},
        {"U'", RubiksCube::MOVE::UPRIME}, {"U2", RubiksCube::MOVE::U2},
        {"L", RubiksCube::MOVE::L}, {"L'", RubiksCube::MOVE::LPRIME},
        {"L2", RubiksCube::MOVE::L2}, {"R", RubiksCube::MOVE::R},
        {"R'", RubiksCube::MOVE::RPRIME}, {"R2", RubiksCube::MOVE::R2},
        {"D", RubiksCube::MOVE::D}, {"D'", RubiksCube::MOVE::DPRIME},
        {"D2", RubiksCube::MOVE::D2}, {"B", RubiksCube::MOVE::B},
        {"B'", RubiksCube::MOVE::BPRIME}, {"B2", RubiksCube::MOVE::B2}
    };

    for (const auto& [name, value] : moves) {
        if (token == name) {
            move = value;
            return true;
        }
    }
    return false;
}

int solveWithCornerDatabase(const std::string& filePath,
                            const std::vector<RubiksCube::MOVE>& scramble,
                            IDAstarOptions options = {}) {
    CornerPatternDatabase database;
    if (!database.fromFile(filePath)) {
        std::cerr << "Could not load a corner database of the expected size from: "
                  << filePath << '\n';
        return 1;
    }

    RubiksCubeBitboard cube;
    for (const auto move : scramble) {
        cube.move(move);
    }
    const CornerPatternHeuristic heuristic(database);
    options.onProgress = [](int bound, std::uint64_t nodes,
                            std::chrono::milliseconds elapsed) {
        std::cout << "IDA* progress: bound=" << bound
                  << ", nodes=" << nodes
                  << ", elapsed=" << elapsed.count() / 1000.0 << " s\n"
                  << std::flush;
    };
    IDAstarSolver<RubiksCubeBitboard,
                  RubiksCubeBitboard::Hash,
                  CornerPatternHeuristic> solver(cube, heuristic, options);
    const auto solution = solver.solve();

    RubiksCubeBitboard verification = cube;
    for (const auto move : solution) {
        verification.move(move);
    }

    std::cout << "Loaded corner database: " << filePath << '\n'
              << "Scramble: ";
    for (const auto move : scramble) {
        std::cout << moveName(move) << ' ';
    }
    std::cout << "\nSolution: ";
    for (const auto move : solution) {
        std::cout << moveName(move) << ' ';
    }
    std::cout << "\nSolution length: " << solution.size()
              << "\nSolved: " << (verification.isSolved() ? "PASS" : "FAIL")
              << "\nSearch status: ";
    switch (solver.getStatus()) {
        case IDAstarStatus::Solved: std::cout << "solved"; break;
        case IDAstarStatus::NodeLimitReached: std::cout << "node limit reached"; break;
        case IDAstarStatus::FrontierLimitReached: std::cout << "frontier limit reached"; break;
        case IDAstarStatus::TimeLimitReached: std::cout << "time limit reached"; break;
        case IDAstarStatus::Exhausted: std::cout << "search exhausted"; break;
        default: std::cout << "not completed"; break;
    }
    std::cout << "\nNodes visited: " << solver.getVisitedNodes()
              << "\nLast IDA* bound: " << solver.getCurrentBound()
              << '\n';
    return solver.getStatus() == IDAstarStatus::Solved && verification.isSolved()
               ? 0
               : 1;
}

int solveWithPriorityQueueAstar(const std::string& filePath,
                                const std::vector<RubiksCube::MOVE>& scramble,
                                IDAstarOptions options = {}) {
    CornerPatternDatabase database;
    if (!database.fromFile(filePath)) {
        std::cerr << "Could not load a corner database of the expected size from: "
                  << filePath << '\n';
        return 1;
    }

    RubiksCubeBitboard cube;
    for (const auto move : scramble) {
        cube.move(move);
    }
    const CornerPatternHeuristic heuristic(database);
    options.onProgress = [](int bound, std::uint64_t nodes,
                            std::chrono::milliseconds elapsed) {
        std::cout << "Priority-queue A* progress: bound=" << bound
                  << ", nodes=" << nodes
                  << ", elapsed=" << elapsed.count() / 1000.0 << " s\n"
                  << std::flush;
    };
    AstarSolver<RubiksCubeBitboard,
                RubiksCubeBitboard::Hash,
                CornerPatternHeuristic> solver(cube, heuristic, options);
    const auto solution = solver.solve();

    RubiksCubeBitboard verification = cube;
    for (const auto move : solution) {
        verification.move(move);
    }

    std::cout << "Loaded corner database: " << filePath << '\n'
              << "Scramble: ";
    for (const auto move : scramble) {
        std::cout << moveName(move) << ' ';
    }
    std::cout << "\nSolution: ";
    for (const auto move : solution) {
        std::cout << moveName(move) << ' ';
    }
    std::cout << "\nSolution length: " << solution.size()
              << "\nSolved: " << (verification.isSolved() ? "PASS" : "FAIL")
              << "\nSearch status: ";
    switch (solver.getStatus()) {
        case IDAstarStatus::Solved: std::cout << "solved"; break;
        case IDAstarStatus::NodeLimitReached: std::cout << "node limit reached"; break;
        case IDAstarStatus::FrontierLimitReached: std::cout << "frontier limit reached"; break;
        case IDAstarStatus::TimeLimitReached: std::cout << "time limit reached"; break;
        case IDAstarStatus::Exhausted: std::cout << "search exhausted"; break;
        default: std::cout << "not completed"; break;
    }
    std::cout << "\nNodes visited: " << solver.getVisitedNodes()
              << "\nPeak frontier: " << solver.getPeakFrontier()
              << "\nLast search bound: " << solver.getCurrentBound()
              << '\n';
    return solver.getStatus() == IDAstarStatus::Solved && verification.isSolved()
               ? 0
               : 1;
}

std::filesystem::path findDepthEightDatabase(const char* executablePath) {
    const std::filesystem::path relativeDatabase =
        std::filesystem::path("Databases") / "cornerDepth8.bin";
    const std::filesystem::path executableDirectory =
        std::filesystem::absolute(executablePath).parent_path();
    const std::array<std::filesystem::path, 3> candidates = {
        relativeDatabase,
        executableDirectory / relativeDatabase,
        executableDirectory / ".." / relativeDatabase
    };

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }
    return relativeDatabase;
}

int runRandomDepthEightSolve(const char* executablePath) {
    constexpr std::size_t scrambleLength = 11;
    constexpr std::size_t scrambleCount = 10;
    const auto databasePath = findDepthEightDatabase(executablePath);

    IDAstarOptions options;
    options.maxNodes = 5'000'000;
    options.maxDuration = std::chrono::seconds(30);
    options.maxOpenStates = 1'000'000;

    std::cout << "===== 10 RANDOM 11-MOVE SCRAMBLES: BITBOARD PRIORITY-QUEUE A* =====\n"
              << "Each solve is limited to 5,000,000 nodes, 30 seconds, or "
                 "1,000,000 open states.\n\n";

    std::size_t solvedCount = 0;
    double totalRuntimeSeconds = 0.0;
    for (std::size_t attempt = 1; attempt <= scrambleCount; ++attempt) {
        const auto scramble = makeRandomScramble(scrambleLength);
        std::cout << "===== Scramble " << attempt << '/' << scrambleCount
                  << " =====\nScramble: ";
        for (const auto move : scramble) {
            std::cout << moveName(move) << ' ';
        }
        std::cout << "\nDatabase: " << databasePath.string() << "\n\n"
                  << std::flush;

        std::cout << "\n----- Priority-queue A* (bitboard) -----\n";
        const auto runStartedAt = std::chrono::steady_clock::now();
        const bool solved =
            solveWithPriorityQueueAstar(databasePath.string(), scramble, options) == 0;
        const double runSeconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - runStartedAt).count();
        totalRuntimeSeconds += runSeconds;
        std::cout << "Run runtime: " << std::fixed << std::setprecision(3)
                  << runSeconds << " seconds\n" << std::defaultfloat;
        if (solved) {
            ++solvedCount;
        }
        std::cout << '\n';
    }

    std::cout << "===== SUMMARY =====\nPriority-queue A* solved: " << solvedCount << '/'
              << scrambleCount << " scrambles\n"
              << "Total runtime: " << std::fixed << std::setprecision(3)
              << totalRuntimeSeconds << " seconds\n"
              << "Average runtime per scramble: "
              << totalRuntimeSeconds / static_cast<double>(scrambleCount)
              << " seconds\n" << std::defaultfloat;
    return solvedCount == scrambleCount ? 0 : 1;
}

int runSingleRandom13PriorityQueueSolve(const char* executablePath) {
    constexpr std::size_t scrambleLength = 13;
    const auto databasePath = findDepthEightDatabase(executablePath);

    IDAstarOptions options;
    options.maxNodes = 1'000'000;
    options.maxDuration = std::chrono::seconds(30);
    options.maxOpenStates = 500'000;

    const auto scramble = makeRandomScramble(scrambleLength);
    std::cout << "===== ONE RANDOM 13-MOVE SCRAMBLE: BITBOARD PRIORITY-QUEUE A* =====\n"
              << "Limits: 1,000,000 nodes, 30 seconds, or 500,000 open states.\n"
              << "Scramble: ";
    for (const auto move : scramble) {
        std::cout << moveName(move) << ' ';
    }
    std::cout << "\nDatabase: " << databasePath.string() << "\n\n" << std::flush;

    const auto startedAt = std::chrono::steady_clock::now();
    const int result = solveWithPriorityQueueAstar(
        databasePath.string(), scramble, options);
    const double seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - startedAt).count();
    std::cout << "Run runtime: " << std::fixed << std::setprecision(3)
              << seconds << " seconds\n" << std::defaultfloat;
    return result;
}

int runFixedSearchComparison(const char* executablePath) {
    const auto databasePath = findDepthEightDatabase(executablePath);
    const std::array<std::vector<RubiksCube::MOVE>, 3> scrambles = {{
        {RubiksCube::MOVE::RPRIME, RubiksCube::MOVE::U, RubiksCube::MOVE::L,
         RubiksCube::MOVE::R2, RubiksCube::MOVE::L, RubiksCube::MOVE::FPRIME,
         RubiksCube::MOVE::R, RubiksCube::MOVE::FPRIME, RubiksCube::MOVE::U2,
         RubiksCube::MOVE::LPRIME, RubiksCube::MOVE::UPRIME},
        {RubiksCube::MOVE::U2, RubiksCube::MOVE::FPRIME, RubiksCube::MOVE::D,
         RubiksCube::MOVE::R2, RubiksCube::MOVE::F2, RubiksCube::MOVE::BPRIME,
         RubiksCube::MOVE::U, RubiksCube::MOVE::D, RubiksCube::MOVE::F,
         RubiksCube::MOVE::BPRIME, RubiksCube::MOVE::L},
        {RubiksCube::MOVE::LPRIME, RubiksCube::MOVE::D2, RubiksCube::MOVE::U2,
         RubiksCube::MOVE::LPRIME, RubiksCube::MOVE::DPRIME,
         RubiksCube::MOVE::UPRIME, RubiksCube::MOVE::RPRIME,
         RubiksCube::MOVE::D2, RubiksCube::MOVE::F, RubiksCube::MOVE::R,
         RubiksCube::MOVE::F}
    }};

    IDAstarOptions options;
    options.maxNodes = std::numeric_limits<std::uint64_t>::max();
    options.maxDuration = std::chrono::seconds(15);
    options.maxOpenStates = 1'000'000;

    std::cout << "===== FIXED-SCRAMBLE SEARCH COMPARISON =====\n"
              << "Both solvers use bitboard + cornerDepth8, same-face pruning, "
                 "and a 15-second time limit.\n"
              << "Priority queue additionally has a 1,000,000-state frontier cap.\n"
              << "Database: " << databasePath.string() << "\n\n";

    std::size_t priorityQueueSolved = 0;
    std::size_t idaStarSolved = 0;
    for (std::size_t index = 0; index < scrambles.size(); ++index) {
        std::cout << "===== Scramble " << index + 1 << '/' << scrambles.size()
                  << " =====\nScramble: ";
        for (const auto move : scrambles[index]) {
            std::cout << moveName(move) << ' ';
        }
        std::cout << "\n\n----- Priority-queue search -----\n" << std::flush;
        auto startedAt = std::chrono::steady_clock::now();
        const bool pqSolved = solveWithPriorityQueueAstar(
            databasePath.string(), scrambles[index], options) == 0;
        const double pqSeconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - startedAt).count();
        std::cout << "Priority-queue runtime: " << std::fixed
                  << std::setprecision(3) << pqSeconds << " seconds\n"
                  << std::defaultfloat;
        priorityQueueSolved += pqSolved ? 1 : 0;

        std::cout << "\n----- Recursive IDA* -----\n" << std::flush;
        startedAt = std::chrono::steady_clock::now();
        const bool idaSolved = solveWithCornerDatabase(
            databasePath.string(), scrambles[index], options) == 0;
        const double idaSeconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - startedAt).count();
        std::cout << "Recursive IDA* runtime: " << std::fixed
                  << std::setprecision(3) << idaSeconds << " seconds\n\n"
                  << std::defaultfloat;
        idaStarSolved += idaSolved ? 1 : 0;
    }

    std::cout << "===== COMPARISON SUMMARY =====\n"
              << "Priority-queue solved: " << priorityQueueSolved << '/'
              << scrambles.size() << "\nRecursive IDA* solved: "
              << idaStarSolved << '/' << scrambles.size() << '\n';
    return priorityQueueSolved == scrambles.size() &&
                   idaStarSolved == scrambles.size()
               ? 0
               : 1;
}

int makeCornerDatabaseFromArguments(int argc, char* argv[]) {
    if (argc != 4 && argc != 6) {
        std::cerr << "Usage: cubiq --make-corner-db <max-depth 0..14> "
                     "<output-file> [--max-frontier <states up to 25000000>]\n";
        return 2;
    }

    unsigned int depth = 0;
    const std::string depthText(argv[2]);
    const auto parsed = std::from_chars(depthText.data(),
                                        depthText.data() + depthText.size(), depth);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != depthText.data() + depthText.size() || depth > 14) {
        std::cerr << "Maximum depth must be an integer from 0 to 14.\n";
        return 2;
    }

    std::size_t maxFrontier = CornerDBMaker::DEFAULT_MAX_FRONTIER;
    if (argc == 6) {
        if (std::string(argv[4]) != "--max-frontier") {
            std::cerr << "Expected --max-frontier after the output file.\n";
            return 2;
        }
        const std::string frontierText(argv[5]);
        const auto frontierParsed = std::from_chars(
            frontierText.data(), frontierText.data() + frontierText.size(),
            maxFrontier);
        if (frontierParsed.ec != std::errc{} ||
            frontierParsed.ptr != frontierText.data() + frontierText.size() ||
            maxFrontier == 0 || maxFrontier > 25'000'000) {
            std::cerr << "Maximum frontier must be an integer from 1 to 25000000.\n";
            return 2;
        }
    }

    try {
        CornerPatternDatabase database;
        CornerDBMaker maker(maxFrontier);
        const auto stats = maker.generate(database, static_cast<std::uint8_t>(depth));
        const std::filesystem::path outputPath(argv[3]);
        if (outputPath.has_parent_path()) {
            std::filesystem::create_directories(outputPath.parent_path());
        }
        database.toFile(outputPath.string());
        std::cout << "Corner database saved to: " << outputPath.string() << '\n'
                  << "Depth computed: " << depth << '\n'
                  << "Corner states discovered: " << stats.discoveredStates << '\n'
                  << "States expanded: " << stats.expandedStates << '\n'
                  << "Peak BFS frontier: " << stats.peakFrontier << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Corner database generation failed: " << error.what() << '\n';
        return 1;
    }
}


int solveWithCornerDatabaseFromArguments(int argc, char* argv[]) {

        if (argc < 3) {
            std::cerr << "Usage: cubiq --solve-with-corner-db <database-file> "
                         "[--max-nodes N] [--max-seconds N] [scramble moves...]\n"
                      << "Example: cubiq --solve-with-corner-db Databases/cornerDepth5.bin "
                         "--max-nodes 5000000 --max-seconds 30 \"F U R D L B\"\n";
            return 2;
        }

        std::vector<std::string> scrambleTokens;
        IDAstarOptions options;
        for (int argument = 3; argument < argc; ++argument) {
            const std::string argumentText(argv[argument]);
            if (argumentText == "--max-nodes" || argumentText == "--max-seconds") {
                if (argument + 1 >= argc) {
                    std::cerr << "Missing value after " << argumentText << ".\n";
                    return 2;
                }
                const std::string valueText(argv[++argument]);
                std::uint64_t value = 0;
                const auto parsed = std::from_chars(
                    valueText.data(), valueText.data() + valueText.size(), value);
                if (parsed.ec != std::errc{} ||
                    parsed.ptr != valueText.data() + valueText.size() ||
                    (argumentText == "--max-seconds" &&
                     value > static_cast<std::uint64_t>(
                         std::chrono::seconds::max().count()))) {
                    std::cerr << "Invalid non-negative integer for "
                              << argumentText << ": " << valueText << '\n';
                    return 2;
                }
                if (argumentText == "--max-nodes") {
                    options.maxNodes = value;
                } else {
                    options.maxDuration = std::chrono::seconds(value);
                }
                continue;
            }
            std::istringstream input(argv[argument]);
            std::string token;
            while (input >> token) {
                scrambleTokens.push_back(token);
            }
        }
        // Preserve the original no-scramble invocation as a small smoke test.
        if (scrambleTokens.empty()) {
            scrambleTokens = {"F", "U", "R"};
        }

        std::vector<RubiksCube::MOVE> scramble;
        scramble.reserve(scrambleTokens.size());
        for (const auto& token : scrambleTokens) {
            RubiksCube::MOVE move;
            if (!parseMoveToken(token, move)) {
                std::cerr << "Invalid move token: " << token
                          << " (use F, F', F2, U, U', U2, L, L', L2, R, R', R2, D, D', D2, B, B', or B2)\n";
                return 2;
            }
            scramble.push_back(move);
        }
        return solveWithCornerDatabase(argv[2], scramble, options);
}
