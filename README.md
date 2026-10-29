# Cubiq

Cubiq is a C++20 project that models and solves a 3x3 Rubik's Cube. It explores several cube representations and compares search strategies, from basic graph traversal to heuristic-guided search.

## Features

- Three cube models: 3D array, 1D array, and bitboard.
- Standard face turns and inverse turns, with state equality and custom hashing.
- Generic DFS, BFS, IDDFS, and IDA* solver implementations.
- Bitboard priority-queue A* search guided by a corner pattern database.
- A corner pattern database with permutation/orientation indexing and packed 4-bit distance storage.
- Tests for move behavior, cube representations, pattern database storage, and solver results.

## Build

Requirements: CMake 3.20 or newer and a C++20 compiler. No external libraries are required.

From the repository root:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable is usually `build/cubiq.exe` with a single-configuration generator, or `build/Release/cubiq.exe` with Visual Studio. Use the corresponding path below.

## Run

Run the project checks:

```powershell
.\build\cubiq.exe --test
```

Run the default randomized benchmark (10 scrambles generated with 11 moves each):

```powershell
.\build\cubiq.exe
```

Run one randomized 13-move scramble or compare the two search approaches on fixed scrambles:

```powershell
.\build\cubiq.exe --random-pq-13
.\build\cubiq.exe --compare-searches
```

The default benchmark and search commands have node, time, and/or frontier caps. A capped search can stop without finding a solution; this keeps long searches bounded. Random scramble length is the number of turns used to generate the input, not necessarily the shortest solution length.

## Solve a supplied scramble

The included corner database is `Databases/cornerDepth8.bin`. From the repository root, pass a scramble as move tokens:

```powershell
.\build\cubiq.exe --solve-with-corner-db Databases/cornerDepth8.bin "F U R"
```

Optional search limits can be supplied before the moves:

```powershell
.\build\cubiq.exe --solve-with-corner-db Databases/cornerDepth8.bin --max-nodes 5000000 --max-seconds 30 "F U R"
```

Supported face turns are `F`, `U`, `L`, `R`, `D`, and `B`; append `'` for an inverse turn or `2` for a half turn.

## Project layout

- `include/` — cube model interfaces and representation headers.
- `src/` — cube implementations and command-line operations.
- `Solver/` — graph search and heuristic solver implementations.
- `PatternDatabases/` — database storage, indexing, and generation.
- `Databases/` — included corner database files.
- `tests/` — project checks.
