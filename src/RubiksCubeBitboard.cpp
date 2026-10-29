#include "RubiksCubeBitboard.h"

namespace {

std::size_t hashCube(const RubiksCube& cube) noexcept {
    std::size_t seed = 0;

    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                const auto color = static_cast<std::size_t>(
                    static_cast<int>(cube.getColor(
                        static_cast<RubiksCube::FACE>(face), row, col))
                );

                seed ^= color + 0x9e3779b97f4a7c15ULL +
                        (seed << 6) + (seed >> 2);
            }
        }
    }

    return seed;
}

}

bool RubiksCubeBitboard::operator==(const RubiksCubeBitboard& other) const {
    for (int face = 0; face < 6; face++) {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                const auto currentFace = static_cast<FACE>(face);

                if (getColor(currentFace, row, col) !=
                    other.getColor(currentFace, row, col)) {
                    return false;
                }
            }
        }
    }

    return true;
}

std::size_t RubiksCubeBitboard::Hash::operator()(
    const RubiksCubeBitboard& cube
) const noexcept {
    return hashCube(cube);
}

RubiksCubeBitboard::RubiksCubeBitboard() {

    // Internal bitboard order is kept stable for the move tables:
    // 0 = UP, 1 = LEFT, 2 = FRONT, 3 = RIGHT, 4 = BACK, 5 = DOWN.

    COLOR colors[6] = {
        COLOR::WHITE,
        COLOR::GREEN,
        COLOR::RED,
        COLOR::BLUE,
        COLOR::ORANGE,
        COLOR::YELLOW
    };

    for (int face = 0; face < 6; face++) {

        bitboard[face] = 0;

        for (int index = 0; index < 8; index++) {
            setSticker(face, index, colors[face]);
        }
    }
}


// ============================================================
// Sticker helpers
// ============================================================

RubiksCube::COLOR RubiksCubeBitboard::getSticker(
    int face,
    int index
) const {

    uint64_t value =
        (bitboard[face] >> (index * 8)) & 0xFFULL;

    return static_cast<COLOR>(value);
}


void RubiksCubeBitboard::setSticker(
    int face,
    int index,
    COLOR color
) {

    uint64_t mask =
        0xFFULL << (index * 8);

    bitboard[face] &= ~mask;

    bitboard[face] |=
        static_cast<uint64_t>(color) << (index * 8);
}


// ============================================================
// Get color
//
// Sticker layout:
//
// 0 1 2
// 7   3
// 6 5 4
// ============================================================

RubiksCube::COLOR RubiksCubeBitboard::getColor(
    FACE face,
    int row,
    int col
) const {

    // RubiksCube::FACE is ordered UP, DOWN, LEFT, RIGHT, FRONT, BACK;
    // the bitboard move tables use UP, LEFT, FRONT, RIGHT, BACK, DOWN.
    const int faceIndex = [&] {
        switch (face) {
            case FACE::UP:    return 0;
            case FACE::LEFT:  return 1;
            case FACE::FRONT: return 2;
            case FACE::RIGHT: return 3;
            case FACE::BACK:  return 4;
            case FACE::DOWN:  return 5;
        }
        return 0;
    }();

    // Center sticker is not stored.
    if (row == 1 && col == 1) {

        COLOR centers[6] = {
            COLOR::WHITE,
            COLOR::GREEN,
            COLOR::RED,
            COLOR::BLUE,
            COLOR::ORANGE,
            COLOR::YELLOW
        };

        return centers[faceIndex];
    }

    int index;

    if (row == 0 && col == 0)
        index = 0;
    else if (row == 0 && col == 1)
        index = 1;
    else if (row == 0 && col == 2)
        index = 2;
    else if (row == 1 && col == 2)
        index = 3;
    else if (row == 2 && col == 2)
        index = 4;
    else if (row == 2 && col == 1)
        index = 5;
    else if (row == 2 && col == 0)
        index = 6;
    else
        index = 7;

    return getSticker(faceIndex, index);
}


// ============================================================
// Solved
// ============================================================

bool RubiksCubeBitboard::isSolved() const {

    COLOR colors[6] = {
        COLOR::WHITE,
        COLOR::GREEN,
        COLOR::RED,
        COLOR::BLUE,
        COLOR::ORANGE,
        COLOR::YELLOW
    };

    for (int face = 0; face < 6; face++) {

        for (int index = 0; index < 8; index++) {

            if (getSticker(face, index) != colors[face]) {
                return false;
            }
        }
    }

    return true;
}


// ============================================================
// Rotate a face clockwise
//
// Layout:
//
// 0 1 2
// 7   3
// 6 5 4
//
// Clockwise:
//
// old 6 -> new 0
// old 7 -> new 1
// old 0 -> new 2
// old 1 -> new 3
// old 2 -> new 4
// old 3 -> new 5
// old 4 -> new 6
// old 5 -> new 7
//
// Since every sticker occupies 8 bits,
// this is a rotation by 16 bits.
// ============================================================

void RubiksCubeBitboard::rotateFaceClockwise(FACE face) {

    const int f = [&] {
        switch (face) {
            case FACE::UP:    return 0;
            case FACE::LEFT:  return 1;
            case FACE::FRONT: return 2;
            case FACE::RIGHT: return 3;
            case FACE::BACK:  return 4;
            case FACE::DOWN:  return 5;
        }
        return 0;
    }();

    uint64_t old = bitboard[f];

    // With clockwise sticker indices, new index 0 receives old index 6.
    bitboard[f] =
        (old << 16) |
        (old >> 48);
}


// ============================================================
// F
// ============================================================

RubiksCube& RubiksCubeBitboard::F() {

    rotateFaceClockwise(FACE::FRONT);

    COLOR temp[3];

    // Save UP bottom row: [2][0..2] -> [6, 5, 4]
    temp[0] = getSticker(0, 6);
    temp[1] = getSticker(0, 5);
    temp[2] = getSticker(0, 4);

    // LEFT -> UP
    setSticker(0, 6, getSticker(1, 4));
    setSticker(0, 5, getSticker(1, 3));
    setSticker(0, 4, getSticker(1, 2));

    // DOWN -> LEFT
    setSticker(1, 2, getSticker(5, 0));
    setSticker(1, 3, getSticker(5, 1));
    setSticker(1, 4, getSticker(5, 2));

    // RIGHT -> DOWN
    setSticker(5, 0, getSticker(3, 6));
    setSticker(5, 1, getSticker(3, 7));
    setSticker(5, 2, getSticker(3, 0));

    // UP -> RIGHT
    setSticker(3, 0, temp[0]);
    setSticker(3, 7, temp[1]);
    setSticker(3, 6, temp[2]);

    return *this;
}


RubiksCube& RubiksCubeBitboard::FPrime() {

    // Apply the inverse strip cycle explicitly. Keeping this independent
    // from F() makes the direction of each facelet transfer unambiguous.
    COLOR temp[3];
    temp[0] = getSticker(0, 6);
    temp[1] = getSticker(0, 5);
    temp[2] = getSticker(0, 4);

    // UP <- RIGHT
    setSticker(0, 6, getSticker(3, 0));
    setSticker(0, 5, getSticker(3, 7));
    setSticker(0, 4, getSticker(3, 6));

    // RIGHT <- DOWN
    setSticker(3, 0, getSticker(5, 2));
    setSticker(3, 7, getSticker(5, 1));
    setSticker(3, 6, getSticker(5, 0));

    // DOWN <- LEFT
    setSticker(5, 0, getSticker(1, 2));
    setSticker(5, 1, getSticker(1, 3));
    setSticker(5, 2, getSticker(1, 4));

    // LEFT <- saved UP
    setSticker(1, 2, temp[2]);
    setSticker(1, 3, temp[1]);
    setSticker(1, 4, temp[0]);

    // Rotate the FRONT face counterclockwise (three clockwise rotations).
    rotateFaceClockwise(FACE::FRONT);
    rotateFaceClockwise(FACE::FRONT);
    rotateFaceClockwise(FACE::FRONT);

    return *this;
}


RubiksCube& RubiksCubeBitboard::F2() {

    F();
    F();

    return *this;
}


// ============================================================
// U
// ============================================================

RubiksCube& RubiksCubeBitboard::U() {

    rotateFaceClockwise(FACE::UP);

    COLOR temp[3];

    // Save FRONT top
    temp[0] = getSticker(2, 0);
    temp[1] = getSticker(2, 1);
    temp[2] = getSticker(2, 2);

    // RIGHT -> FRONT
    setSticker(2, 0, getSticker(3, 0));
    setSticker(2, 1, getSticker(3, 1));
    setSticker(2, 2, getSticker(3, 2));

    // BACK -> RIGHT
    setSticker(3, 0, getSticker(4, 0));
    setSticker(3, 1, getSticker(4, 1));
    setSticker(3, 2, getSticker(4, 2));

    // LEFT -> BACK
    setSticker(4, 0, getSticker(1, 0));
    setSticker(4, 1, getSticker(1, 1));
    setSticker(4, 2, getSticker(1, 2));

    // FRONT -> LEFT
    setSticker(1, 0, temp[0]);
    setSticker(1, 1, temp[1]);
    setSticker(1, 2, temp[2]);

    return *this;
}


RubiksCube& RubiksCubeBitboard::UPrime() {

    U();
    U();
    U();

    return *this;
}


RubiksCube& RubiksCubeBitboard::U2() {

    U();
    U();

    return *this;
}


// ============================================================
// L
// ============================================================

RubiksCube& RubiksCubeBitboard::L() {

    rotateFaceClockwise(FACE::LEFT);

    COLOR temp[3];

    // Save UP left column
    temp[0] = getSticker(0, 0);
    temp[1] = getSticker(0, 7);
    temp[2] = getSticker(0, 6);

    // BACK -> UP
    setSticker(0, 0, getSticker(4, 4));
    setSticker(0, 7, getSticker(4, 3));
    setSticker(0, 6, getSticker(4, 2));

    // DOWN -> BACK
    setSticker(4, 4, getSticker(5, 0));
    setSticker(4, 3, getSticker(5, 7));
    setSticker(4, 2, getSticker(5, 6));

    // FRONT -> DOWN
    setSticker(5, 0, getSticker(2, 0));
    setSticker(5, 7, getSticker(2, 7));
    setSticker(5, 6, getSticker(2, 6));

    // UP -> FRONT
    setSticker(2, 0, temp[0]);
    setSticker(2, 7, temp[1]);
    setSticker(2, 6, temp[2]);

    return *this;
}


RubiksCube& RubiksCubeBitboard::LPrime() {

    L();
    L();
    L();

    return *this;
}


RubiksCube& RubiksCubeBitboard::L2() {

    L();
    L();

    return *this;
}


// ============================================================
// R
// ============================================================

RubiksCube& RubiksCubeBitboard::R() {

    rotateFaceClockwise(FACE::RIGHT);

    COLOR temp[3];

    // Save UP right column
    temp[0] = getSticker(0, 2);
    temp[1] = getSticker(0, 3);
    temp[2] = getSticker(0, 4);

    // FRONT -> UP
    setSticker(0, 2, getSticker(2, 2));
    setSticker(0, 3, getSticker(2, 3));
    setSticker(0, 4, getSticker(2, 4));

    // DOWN -> FRONT
    setSticker(2, 2, getSticker(5, 2));
    setSticker(2, 3, getSticker(5, 3));
    setSticker(2, 4, getSticker(5, 4));

    // BACK -> DOWN
    setSticker(5, 2, getSticker(4, 6));
    setSticker(5, 3, getSticker(4, 7));
    setSticker(5, 4, getSticker(4, 0));

    // UP -> BACK
    setSticker(4, 6, temp[0]);
    setSticker(4, 7, temp[1]);
    setSticker(4, 0, temp[2]);

    return *this;
}


RubiksCube& RubiksCubeBitboard::RPrime() {

    R();
    R();
    R();

    return *this;
}


RubiksCube& RubiksCubeBitboard::R2() {

    R();
    R();

    return *this;
}


// ============================================================
// D
// ============================================================

RubiksCube& RubiksCubeBitboard::D() {

    rotateFaceClockwise(FACE::DOWN);

    COLOR temp[3];

    // Save FRONT bottom
    temp[0] = getSticker(2, 6);
    temp[1] = getSticker(2, 5);
    temp[2] = getSticker(2, 4);

    // LEFT -> FRONT
    setSticker(2, 6, getSticker(1, 6));
    setSticker(2, 5, getSticker(1, 5));
    setSticker(2, 4, getSticker(1, 4));

    // BACK -> LEFT
    setSticker(1, 6, getSticker(4, 6));
    setSticker(1, 5, getSticker(4, 5));
    setSticker(1, 4, getSticker(4, 4));

    // RIGHT -> BACK
    setSticker(4, 6, getSticker(3, 6));
    setSticker(4, 5, getSticker(3, 5));
    setSticker(4, 4, getSticker(3, 4));

    // FRONT -> RIGHT
    setSticker(3, 6, temp[0]);
    setSticker(3, 5, temp[1]);
    setSticker(3, 4, temp[2]);

    return *this;
}


RubiksCube& RubiksCubeBitboard::DPrime() {

    D();
    D();
    D();

    return *this;
}


RubiksCube& RubiksCubeBitboard::D2() {

    D();
    D();

    return *this;
}


// ============================================================
// B
// ============================================================

RubiksCube& RubiksCubeBitboard::B() {

    rotateFaceClockwise(FACE::BACK);

    COLOR temp[3];

    // Save UP top
    temp[0] = getSticker(0, 0);
    temp[1] = getSticker(0, 1);
    temp[2] = getSticker(0, 2);

    // RIGHT -> UP
    setSticker(0, 0, getSticker(3, 2));
    setSticker(0, 1, getSticker(3, 3));
    setSticker(0, 2, getSticker(3, 4));

    // DOWN -> RIGHT
    setSticker(3, 4, getSticker(5, 6));
    setSticker(3, 3, getSticker(5, 5));
    setSticker(3, 2, getSticker(5, 4));

    // LEFT -> DOWN
    setSticker(5, 6, getSticker(1, 0));
    setSticker(5, 5, getSticker(1, 7));
    setSticker(5, 4, getSticker(1, 6));

    // UP -> LEFT
    setSticker(1, 6, temp[0]);
    setSticker(1, 7, temp[1]);
    setSticker(1, 0, temp[2]);

    return *this;
}


RubiksCube& RubiksCubeBitboard::BPrime() {

    B();
    B();
    B();

    return *this;
}


RubiksCube& RubiksCubeBitboard::B2() {

    B();
    B();

    return *this;
}
