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
class TestPatternDatabase final : public PatternDatabase {
public:
    using PatternDatabase::PatternDatabase;

    std::uint32_t getDatabaseIndex(const RubiksCube&) const override {
        return 0;
    }
};

bool testNibbleArray() {
    NibbleArray values(5, 0x0F);
    for (std::size_t index = 0; index < values.size(); ++index) {
        values.set(index, static_cast<std::uint8_t>(index));
    }

    if (values.storageSize() != 3) {
        return false;
    }
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (values.get(index) != index) {
            return false;
        }
    }

    values.set(0, 0x0A);
    values.set(1, 0x05);
    if (values.get(0) != 0x0A || values.get(1) != 0x05) {
        return false;
    }

    values.reset(0x0F);
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (values.get(index) != 0x0F) {
            return false;
        }
    }
    return true;
}

bool testPatternDatabase() {
    TestPatternDatabase database(4);
    RubiksCubeBitboard cube;

    if (database.getSize() != 4 || database.getNumItems() != 0 ||
        database.isFull() || database.getNumMoves(cube) != 0x0F) {
        return false;
    }

    if (!database.setNumMoves(cube, 3) ||
        database.setNumMoves(cube, 5) ||
        database.getNumMoves(cube) != 3 ||
        database.getNumItems() != 1) {
        return false;
    }

    database.setNumMoves(1, 7);
    database.setNumMoves(2, 2);
    database.setNumMoves(3, 11);
    if (!database.isFull() || database.getNumItems() != 4) {
        return false;
    }

    const auto uniqueSuffix = std::chrono::steady_clock::now()
                                  .time_since_epoch().count();
    const auto filePath = std::filesystem::temp_directory_path() /
        ("cubiq_pattern_database_test_" + std::to_string(uniqueSuffix) + ".bin");
    database.toFile(filePath.string());

    TestPatternDatabase loaded(4);
    const bool loadedSuccessfully = loaded.fromFile(filePath.string());
    std::error_code removeError;
    std::filesystem::remove(filePath, removeError);

    if (!loadedSuccessfully || !loaded.isFull() || loaded.getNumItems() != 4) {
        return false;
    }
    return loaded.getNumMoves(0) == 3 &&
           loaded.getNumMoves(1) == 7 &&
           loaded.getNumMoves(2) == 2 &&
           loaded.getNumMoves(3) == 11;
}

bool testCornerMetadata(RubiksCube& cube, bool expectSolvedLayout) {
    constexpr std::uint8_t solvedCornerIndices[8] = {
        0, 1, 3, 2, 4, 5, 6, 7
    };
    bool seen[8] = {};
    unsigned orientationSum = 0;

    for (std::uint8_t position = 0; position < 8; ++position) {
        const std::uint8_t cornerIndex = cube.getCornerIndex(position);
        const std::uint8_t orientation = cube.getCornerOrientation(position);
        if (cornerIndex >= 8 || seen[cornerIndex] || orientation > 2) {
            return false;
        }
        seen[cornerIndex] = true;
        orientationSum += orientation;

        if (expectSolvedLayout &&
            (cornerIndex != solvedCornerIndices[position] || orientation != 0)) {
            return false;
        }
    }

    return orientationSum % 3 == 0;
}

std::string moveName(RubiksCube::MOVE move);

bool testTwoMoveCornerTransitions() {
    constexpr const char* representationNames[] = {"3D array", "1D array", "Bitboard"};
    CornerPatternDatabase database;
    for (int firstMove = 0; firstMove < 18; ++firstMove) {
        for (int secondMove = 0; secondMove < 18; ++secondMove) {
            RubiksCube3DArray cube3D;
            RubiksCube1DArray cube1D;
            RubiksCubeBitboard cubeBitboard;
            RubiksCube* cubes[] = {
                static_cast<RubiksCube*>(&cube3D),
                static_cast<RubiksCube*>(&cube1D),
                static_cast<RubiksCube*>(&cubeBitboard)
            };
            for (std::size_t representation = 0; representation < 3; ++representation) {
                RubiksCube* cube = cubes[representation];
                cube->move(static_cast<RubiksCube::MOVE>(firstMove));
                cube->move(static_cast<RubiksCube::MOVE>(secondMove));
                bool metadataValid = false;
                try {
                    metadataValid = testCornerMetadata(*cube, false);
                } catch (const std::exception& error) {
                    std::cerr << "Corner metadata exception: "
                              << representationNames[representation] << " after "
                              << moveName(static_cast<RubiksCube::MOVE>(firstMove)) << ' '
                              << moveName(static_cast<RubiksCube::MOVE>(secondMove))
                              << ": " << error.what() << '\n';
                    if (representation == 2) {
                        for (std::uint8_t position = 0; position < 8; ++position) {
                            std::cerr << "  position " << static_cast<int>(position)
                                      << ": cubie="
                                      << static_cast<int>(cube->getCornerIndex(position))
                                      << '\n';
                        }
                        std::cerr << "  Bitboard vs 3D facelet differences:\n";
                        for (int face = 0; face < 6; ++face) {
                            for (int row = 0; row < 3; ++row) {
                                for (int col = 0; col < 3; ++col) {
                                    const auto faceId = static_cast<RubiksCube::FACE>(face);
                                    if (cube->getColor(faceId, row, col) !=
                                        cube3D.getColor(faceId, row, col)) {
                                        std::cerr << "    face=" << face << ", row=" << row
                                                  << ", col=" << col << ": bitboard="
                                                  << RubiksCube::getColorLetter(
                                                         cube->getColor(faceId, row, col))
                                                  << ", 3D="
                                                  << RubiksCube::getColorLetter(
                                                         cube3D.getColor(faceId, row, col))
                                                  << '\n';
                                    }
                                }
                            }
                        }
                    }
                    return false;
                }
                if (!metadataValid) {
                    std::cerr << "Corner metadata failure: " << representationNames[representation]
                              << " after "
                              << moveName(static_cast<RubiksCube::MOVE>(firstMove)) << ' '
                              << moveName(static_cast<RubiksCube::MOVE>(secondMove)) << '\n';
                    for (std::uint8_t position = 0; position < 8; ++position) {
                        std::cerr << "  position " << static_cast<int>(position)
                                  << ": cubie=" << static_cast<int>(cube->getCornerIndex(position))
                                  << ", orientation="
                                  << static_cast<int>(cube->getCornerOrientation(position)) << '\n';
                    }
                    if (representation == 2) {
                        std::cerr << "  Bitboard vs 3D facelet differences:\n";
                        for (int face = 0; face < 6; ++face) {
                            for (int row = 0; row < 3; ++row) {
                                for (int col = 0; col < 3; ++col) {
                                    const auto faceId = static_cast<RubiksCube::FACE>(face);
                                    if (cube->getColor(faceId, row, col) !=
                                        cube3D.getColor(faceId, row, col)) {
                                        std::cerr << "    face=" << face << ", row=" << row
                                                  << ", col=" << col << ": bitboard="
                                                  << RubiksCube::getColorLetter(
                                                         cube->getColor(faceId, row, col))
                                                  << ", 3D="
                                                  << RubiksCube::getColorLetter(
                                                         cube3D.getColor(faceId, row, col))
                                                  << '\n';
                                    }
                                }
                            }
                        }
                    }
                    return false;
                }
            }

            const auto index3D = database.getDatabaseIndex(cube3D);
            const auto index1D = database.getDatabaseIndex(cube1D);
            const auto indexBitboard = database.getDatabaseIndex(cubeBitboard);
            if (index3D != index1D || index3D != indexBitboard) {
                std::cerr << "Corner-index mismatch after "
                          << moveName(static_cast<RubiksCube::MOVE>(firstMove)) << ' '
                          << moveName(static_cast<RubiksCube::MOVE>(secondMove))
                          << ": 3D=" << index3D << ", 1D=" << index1D
                          << ", bitboard=" << indexBitboard << '\n';
                return false;
            }
        }
    }
    return true;
}

bool testCornerPatternIndex(const RubiksCube& cube3D,
                            const RubiksCube& cube1D,
                            const RubiksCube& cubeBitboard) {
    CornerPatternDatabase database;
    RubiksCube3DArray solvedCube;
    const std::uint32_t index3D = database.getDatabaseIndex(cube3D);
    const std::uint32_t index1D = database.getDatabaseIndex(cube1D);
    const std::uint32_t indexBitboard = database.getDatabaseIndex(cubeBitboard);
    const std::uint32_t solvedIndex = database.getDatabaseIndex(solvedCube);

    PermutationIndexer<8> permutationIndexer;
    const std::array<std::uint8_t, 8> solvedCornerIds = {
        0, 1, 3, 2, 4, 5, 6, 7
    };

    if (database.getSize() != CornerPatternDatabase::DATABASE_SIZE ||
        index3D >= database.getSize() || index3D != index1D ||
        index3D != indexBitboard ||
        permutationIndexer.rank(solvedCornerIds) != 120 ||
        solvedIndex != 120 * CornerPatternDatabase::ORIENTATION_COUNT) {
        return false;
    }

    return true;
}

template<typename T, typename H, typename Heuristic>
bool testIDAStar(const char* representation,
                 const T& scrambled,
                 const Heuristic& heuristic) {
    IDAstarSolver<T, H, Heuristic> solver(scrambled, heuristic);
    const std::vector<RubiksCube::MOVE> solution = solver.solve();

    T verification = scrambled;
    for (const auto move : solution) {
        verification.move(move);
    }

    const bool passed = verification.isSolved();
    std::cout << representation << ": " << (passed ? "PASS" : "FAIL")
              << " | solution length: " << solution.size() << " | moves: ";
    if (solution.empty()) {
        std::cout << (passed ? "<already solved>" : "<none>");
    } else {
        for (const auto move : solution) {
            std::cout << moveName(move) << ' ';
        }
    }
    std::cout << '\n';
    return passed;
}

bool createAndLoadTestCornerDatabase(CornerPatternDatabase& loadedDatabase) {
    const auto uniqueSuffix = std::chrono::steady_clock::now()
                                  .time_since_epoch().count();
    const auto filePath = std::filesystem::temp_directory_path() /
        ("cubiq_corner_database_test_" + std::to_string(uniqueSuffix) + ".bin");

    try {
        CornerPatternDatabase generated;
        CornerDBMaker maker;
        const auto stats = maker.generate(generated, 3);
        generated.toFile(filePath.string());
        const bool loaded = loadedDatabase.fromFile(filePath.string());
        std::error_code removeError;
        std::filesystem::remove(filePath, removeError);

        std::cout << "Corner DB generation (depth 3): "
                  << (loaded ? "PASS" : "FAIL")
                  << " | discovered: " << stats.discoveredStates
                  << " | peak frontier: " << stats.peakFrontier << '\n';
        return loaded &&
               loadedDatabase.getNumMoves(loadedDatabase.getDatabaseIndex(
                   RubiksCube3DArray{})) == 0;
    } catch (const std::exception& error) {
        std::error_code removeError;
        std::filesystem::remove(filePath, removeError);
        std::cout << "Corner DB generation (depth 3): FAIL | "
                  << error.what() << '\n';
        return false;
    }
}


int runProjectTests() {
    const bool nibblePass = testNibbleArray();
    const bool patternDatabasePass = testPatternDatabase();
    std::cout << "===== PATTERN DATABASE STORAGE TEST =====\n";
    std::cout << "NibbleArray packing/indexing/reset: "
              << (nibblePass ? "PASS" : "FAIL") << '\n';
    std::cout << "PatternDatabase set/get/file round-trip: "
              << (patternDatabasePass ? "PASS" : "FAIL") << "\n\n";

    RubiksCube3DArray cube3D;
    RubiksCube1DArray cube1D;
    RubiksCubeBitboard cubeBitboard;

    const bool solvedCornersPass = testCornerMetadata(cube3D, true) &&
                                   testCornerMetadata(cube1D, true) &&
                                   testCornerMetadata(cubeBitboard, true);

    for (auto* cube : {static_cast<RubiksCube*>(&cube3D),
                       static_cast<RubiksCube*>(&cube1D),
                       static_cast<RubiksCube*>(&cubeBitboard)}) {
        cube->F();
        cube->U();
        cube->R();
    }

    const bool scrambledCornersPass = testCornerMetadata(cube3D, false) &&
                                      testCornerMetadata(cube1D, false) &&
                                      testCornerMetadata(cubeBitboard, false);
    const bool twoMoveCornerTransitionsPass = testTwoMoveCornerTransitions();
    const bool cornerPatternIndexPass = testCornerPatternIndex(
        cube3D, cube1D, cubeBitboard);

    std::cout << "===== CORNER INDEX TEST =====\n";
    std::cout << "Solved corner IDs/orientations: "
              << (solvedCornersPass ? "PASS" : "FAIL") << '\n';
    std::cout << "Scrambled corner permutation/orientation laws: "
              << (scrambledCornersPass ? "PASS" : "FAIL") << '\n';
    std::cout << "All 324 two-move corner transitions: "
              << (twoMoveCornerTransitionsPass ? "PASS" : "FAIL") << '\n';
    std::cout << "Corner database index range/representation match: "
              << (cornerPatternIndexPass ? "PASS" : "FAIL") << '\n';

    if (!twoMoveCornerTransitionsPass) {
        std::cout << "Skipping database generation and IDA* because corner transitions are invalid.\n"
                  << "\nOverall: FAIL\n";
        return 1;
    }

    CornerPatternDatabase cornerDatabase;
    const bool cornerDatabaseGenerationPass =
        createAndLoadTestCornerDatabase(cornerDatabase);
    std::cout << "\n";

    if (!cornerDatabaseGenerationPass) {
        std::cout << "IDA* test skipped because the corner database is invalid.\n"
                  << "\nOverall: FAIL\n";
        return 1;
    }

    std::cout << "===== IDA* SOLVER TEST (CORNER DATABASE HEURISTIC) =====\n\n";
    std::cout << "Scramble: F U R\n\n";
    cube3D.print();
    std::cout << '\n';

    const CornerPatternHeuristic heuristic(cornerDatabase);
    const bool pass3D = testIDAStar<RubiksCube3DArray,
                                  RubiksCube3DArray::Hash>("3D array", cube3D, heuristic);
    const bool pass1D = testIDAStar<RubiksCube1DArray,
                                  RubiksCube1DArray::Hash>("1D array", cube1D, heuristic);
    const bool passBitboard = testIDAStar<RubiksCubeBitboard,
                                        RubiksCubeBitboard::Hash>("Bitboard", cubeBitboard,
                                                                  heuristic);

    const bool allPassed = nibblePass && patternDatabasePass &&
                           solvedCornersPass && scrambledCornersPass &&
                           twoMoveCornerTransitionsPass && cornerPatternIndexPass &&
                           cornerDatabaseGenerationPass &&
                           pass3D && pass1D && passBitboard;
    std::cout << "\nOverall: " << (allPassed ? "PASS" : "FAIL") << '\n';
    return allPassed ? 0 : 1;
}
