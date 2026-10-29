cmake_minimum_required(VERSION 3.20)

project(cubiq)

set(CMAKE_CXX_STANDARD 20)

add_executable(cubiq
        main.cpp
        src/ProjectCommands.cpp
        tests/ProjectTests.cpp
        src/RubiksCube.cpp
        include/RubiksCube3DArray.h
        src/RubiksCube3DArray.cpp
        include/RubiksCube1DArray.h
        src/RubiksCube1DArray.cpp
        include/RubiksCubeBitboard.h
        src/RubiksCubeBitboard.cpp
        Solver/DFSSolver.h
        Solver/BFSSolver.h
        Solver/IDDFSSolver.h
        Solver/IDAstarSolver.h
        Solver/AstarSolver.h
        PatternDatabases/NibbleArray.h
        PatternDatabases/NibbleArray.cpp
        PatternDatabases/PatternDatabase.h
        PatternDatabases/PatternDatabase.cpp
        PatternDatabases/math.h
        PatternDatabases/math.cpp
        PatternDatabases/PermutationIndexer.h
        PatternDatabases/CornerPatternDatabase.h
        PatternDatabases/CornerPatternDatabase.cpp
        PatternDatabases/CornerDBMaker.h
        PatternDatabases/CornerDBMaker.cpp
        Solver/CornerPatternHeuristic.h
)

target_include_directories(cubiq PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
