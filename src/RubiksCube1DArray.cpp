#include "RubiksCube1DArray.h"

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

bool RubiksCube1DArray::operator==(const RubiksCube1DArray& other) const {
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

std::size_t RubiksCube1DArray::Hash::operator()(
    const RubiksCube1DArray& cube
) const noexcept {
    return hashCube(cube);
}

RubiksCube1DArray::RubiksCube1DArray() {

    for (int i = 0; i < 54; i++) {

        if (i < 9) {
            cube[i] = COLOR::WHITE;
        }
        else if (i < 18) {
            cube[i] = COLOR::YELLOW;
        }
        else if (i < 27) {
            cube[i] = COLOR::GREEN;
        }
        else if (i < 36) {
            cube[i] = COLOR::BLUE;
        }
        else if (i < 45) {
            cube[i] = COLOR::RED;
        }
        else {
            cube[i] = COLOR::ORANGE;
        }
    }
}


RubiksCube::COLOR RubiksCube1DArray::getColor(
    FACE face,
    int row,
    int col
) const {

    int faceIndex = static_cast<int>(face);

    int index = faceIndex * 9 + row * 3 + col;

    return cube[index];
}


bool RubiksCube1DArray::isSolved() {

    for (int face = 0; face < 6; face++) {

        COLOR center = cube[face * 9 + 4];

        for (int i = 0; i < 9; i++) {

            if (cube[face * 9 + i] != center) {
                return false;
            }
        }
    }

    return true;
}


void RubiksCube1DArray::rotateFaceClockwise(FACE face) {

    int start = static_cast<int>(face) * 9;

    COLOR temp[9];

    for (int i = 0; i < 9; i++) {
        temp[i] = cube[start + i];
    }

    cube[start + 0] = temp[6];
    cube[start + 1] = temp[3];
    cube[start + 2] = temp[0];

    cube[start + 3] = temp[7];
    cube[start + 4] = temp[4];
    cube[start + 5] = temp[1];

    cube[start + 6] = temp[8];
    cube[start + 7] = temp[5];
    cube[start + 8] = temp[2];
}


// ==================== F ====================

RubiksCube& RubiksCube1DArray::F() {

    rotateFaceClockwise(FACE::FRONT);

    COLOR temp[3];

    // Save UP bottom row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP) * 9 + 2 * 3 + i];
    }

    // LEFT -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP) * 9 + 2 * 3 + i] =
            cube[static_cast<int>(FACE::LEFT) * 9 + (2 - i) * 3 + 2];
    }

    // DOWN -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT) * 9 + i * 3 + 2] =
            cube[static_cast<int>(FACE::DOWN) * 9 + i];
    }

    // RIGHT -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN) * 9 + i] =
            cube[static_cast<int>(FACE::RIGHT) * 9 + (2 - i) * 3];
    }

    // UP -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT) * 9 + i * 3] =
            temp[i];
    }

    return *this;
}


RubiksCube& RubiksCube1DArray::FPrime() {

    F();
    F();
    F();

    return *this;
}


RubiksCube& RubiksCube1DArray::F2() {

    F();
    F();

    return *this;
}


// ==================== U ====================

RubiksCube& RubiksCube1DArray::U() {

    rotateFaceClockwise(FACE::UP);

    COLOR temp[3];

    // Save FRONT top row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::FRONT) * 9 + i];
    }

    // RIGHT -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT) * 9 + i] =
            cube[static_cast<int>(FACE::RIGHT) * 9 + i];
    }

    // BACK -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT) * 9 + i] =
            cube[static_cast<int>(FACE::BACK) * 9 + i];
    }

    // LEFT -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK) * 9 + i] =
            cube[static_cast<int>(FACE::LEFT) * 9 + i];
    }

    // FRONT -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT) * 9 + i] =
            temp[i];
    }

    return *this;
}


RubiksCube& RubiksCube1DArray::UPrime() {

    U();
    U();
    U();

    return *this;
}


RubiksCube& RubiksCube1DArray::U2() {

    U();
    U();

    return *this;
}


// ==================== L ====================

RubiksCube& RubiksCube1DArray::L() {

    rotateFaceClockwise(FACE::LEFT);

    COLOR temp[3];

    // Save UP left column
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP) * 9 + i * 3];
    }

    // BACK -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP) * 9 + i * 3] =
            cube[static_cast<int>(FACE::BACK) * 9 + (2 - i) * 3 + 2];
    }

    // DOWN -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK) * 9 + (2 - i) * 3 + 2] =
            cube[static_cast<int>(FACE::DOWN) * 9 + i * 3];
    }

    // FRONT -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN) * 9 + i * 3] =
            cube[static_cast<int>(FACE::FRONT) * 9 + i * 3];
    }

    // UP -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT) * 9 + i * 3] =
            temp[i];
    }

    return *this;
}


RubiksCube& RubiksCube1DArray::LPrime() {

    L();
    L();
    L();

    return *this;
}


RubiksCube& RubiksCube1DArray::L2() {

    L();
    L();

    return *this;
}


// ==================== R ====================

RubiksCube& RubiksCube1DArray::R() {

    rotateFaceClockwise(FACE::RIGHT);

    COLOR temp[3];

    // Save UP right column
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP) * 9 + i * 3 + 2];
    }

    // FRONT -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP) * 9 + i * 3 + 2] =
            cube[static_cast<int>(FACE::FRONT) * 9 + i * 3 + 2];
    }

    // DOWN -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT) * 9 + i * 3 + 2] =
            cube[static_cast<int>(FACE::DOWN) * 9 + i * 3 + 2];
    }

    // BACK -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN) * 9 + i * 3 + 2] =
            cube[static_cast<int>(FACE::BACK) * 9 + (2 - i) * 3];
    }

    // UP -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK) * 9 + (2 - i) * 3] =
            temp[i];
    }

    return *this;
}


RubiksCube& RubiksCube1DArray::RPrime() {

    R();
    R();
    R();

    return *this;
}


RubiksCube& RubiksCube1DArray::R2() {

    R();
    R();

    return *this;
}


// ==================== D ====================

RubiksCube& RubiksCube1DArray::D() {

    rotateFaceClockwise(FACE::DOWN);

    COLOR temp[3];

    // Save FRONT bottom row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::FRONT) * 9 + 6 + i];
    }

    // LEFT -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT) * 9 + 6 + i] =
            cube[static_cast<int>(FACE::LEFT) * 9 + 6 + i];
    }

    // BACK -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT) * 9 + 6 + i] =
            cube[static_cast<int>(FACE::BACK) * 9 + 6 + i];
    }

    // RIGHT -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK) * 9 + 6 + i] =
            cube[static_cast<int>(FACE::RIGHT) * 9 + 6 + i];
    }

    // FRONT -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT) * 9 + 6 + i] =
            temp[i];
    }

    return *this;
}


RubiksCube& RubiksCube1DArray::DPrime() {

    D();
    D();
    D();

    return *this;
}


RubiksCube& RubiksCube1DArray::D2() {

    D();
    D();

    return *this;
}


// ==================== B ====================

RubiksCube& RubiksCube1DArray::B() {

    rotateFaceClockwise(FACE::BACK);

    COLOR temp[3];

    // Save UP top row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP) * 9 + i];
    }

    // RIGHT -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP) * 9 + i] =
            cube[static_cast<int>(FACE::RIGHT) * 9 + i * 3 + 2];
    }

    // DOWN -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT) * 9 + 6 - i * 3 + 2] =
            cube[static_cast<int>(FACE::DOWN) * 9 + 6 + i];
    }

    // LEFT -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN) * 9 + 6 + i] =
            cube[static_cast<int>(FACE::LEFT) * 9 + i * 3];
    }

    // UP -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT) * 9 + 6 - i * 3] =
            temp[i];
    }

    return *this;
}


RubiksCube& RubiksCube1DArray::BPrime() {

    B();
    B();
    B();

    return *this;
}


RubiksCube& RubiksCube1DArray::B2() {

    B();
    B();

    return *this;
}
