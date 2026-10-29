#include "RubiksCube3DArray.h"

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

bool RubiksCube3DArray::operator==(const RubiksCube3DArray& other) const {
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

std::size_t RubiksCube3DArray::Hash::operator()(
    const RubiksCube3DArray& cube
) const noexcept {
    return hashCube(cube);
}

RubiksCube3DArray::RubiksCube3DArray() {

    for (int row = 0; row < 3; row++) {

        for (int col = 0; col < 3; col++) {

            cube[static_cast<int>(FACE::UP)][row][col] =
                COLOR::WHITE;

            cube[static_cast<int>(FACE::DOWN)][row][col] =
                COLOR::YELLOW;

            cube[static_cast<int>(FACE::LEFT)][row][col] =
                COLOR::GREEN;

            cube[static_cast<int>(FACE::RIGHT)][row][col] =
                COLOR::BLUE;

            cube[static_cast<int>(FACE::FRONT)][row][col] =
                COLOR::RED;

            cube[static_cast<int>(FACE::BACK)][row][col] =
                COLOR::ORANGE;
        }
    }
}

RubiksCube::COLOR RubiksCube3DArray::getColor(
    FACE face,
    int row,
    int col
) const {

    return cube[static_cast<int>(face)][row][col];
}

void RubiksCube3DArray::rotateFaceClockwise(FACE face) {

    int f = static_cast<int>(face);

    COLOR temp[3][3];

    // Rotate the face clockwise
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {

            temp[col][2 - row] = cube[f][row][col];
        }
    }

    // Copy rotated face back
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {

            cube[f][row][col] = temp[row][col];
        }
    }
}

RubiksCube& RubiksCube3DArray::F() {

    rotateFaceClockwise(FACE::FRONT);

    COLOR temp[3];

    // Save UP bottom row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP)][2][i];
    }

    // LEFT -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP)][2][i] =
            cube[static_cast<int>(FACE::LEFT)][2 - i][2];
    }

    // DOWN -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT)][i][2] =
            cube[static_cast<int>(FACE::DOWN)][0][i];
    }

    // RIGHT -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN)][0][i] =
            cube[static_cast<int>(FACE::RIGHT)][2 - i][0];
    }

    // UP -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT)][i][0] =
            temp[i];
    }

    return *this;
}

RubiksCube& RubiksCube3DArray::FPrime() {

    F();
    F();
    F();

    return *this;
}

RubiksCube& RubiksCube3DArray::F2() {

    F();
    F();

    return *this;
}


RubiksCube& RubiksCube3DArray::U() {

    rotateFaceClockwise(FACE::UP);

    COLOR temp[3];

    // Save FRONT top row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::FRONT)][0][i];
    }

    // RIGHT -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT)][0][i] =
            cube[static_cast<int>(FACE::RIGHT)][0][i];
    }

    // BACK -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT)][0][i] =
            cube[static_cast<int>(FACE::BACK)][0][i];
    }

    // LEFT -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK)][0][i] =
            cube[static_cast<int>(FACE::LEFT)][0][i];
    }

    // FRONT -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT)][0][i] =
            temp[i];
    }

    return *this;
}

RubiksCube& RubiksCube3DArray::UPrime() {

    U();
    U();
    U();

    return *this;
}

RubiksCube& RubiksCube3DArray::U2() {

    U();
    U();

    return *this;
}

RubiksCube& RubiksCube3DArray::L() {

    // Rotate LEFT face clockwise
    rotateFaceClockwise(FACE::LEFT);

    COLOR temp[3];

    // Save UP left column
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP)][i][0];
    }

    // BACK -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP)][i][0] =
            cube[static_cast<int>(FACE::BACK)][2 - i][2];
    }

    // DOWN -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK)][2 - i][2] =
            cube[static_cast<int>(FACE::DOWN)][i][0];
    }

    // FRONT -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN)][i][0] =
            cube[static_cast<int>(FACE::FRONT)][i][0];
    }

    // UP -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT)][i][0] =
            temp[i];
    }

    return *this;
}

RubiksCube& RubiksCube3DArray::LPrime() {

    L();
    L();
    L();

    return *this;
}

RubiksCube& RubiksCube3DArray::L2() {

    L();
    L();

    return *this;
}

RubiksCube& RubiksCube3DArray::R() {

    // Rotate RIGHT face clockwise
    rotateFaceClockwise(FACE::RIGHT);

    COLOR temp[3];

    // Save UP right column
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP)][i][2];
    }

    // FRONT -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP)][i][2] =
            cube[static_cast<int>(FACE::FRONT)][i][2];
    }

    // DOWN -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT)][i][2] =
            cube[static_cast<int>(FACE::DOWN)][i][2];
    }

    // BACK -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN)][i][2] =
            cube[static_cast<int>(FACE::BACK)][2 - i][0];
    }

    // UP -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK)][2 - i][0] =
            temp[i];
    }

    return *this;
}

RubiksCube& RubiksCube3DArray::RPrime() {

    R();
    R();
    R();

    return *this;
}

RubiksCube& RubiksCube3DArray::R2() {

    R();
    R();

    return *this;
}

RubiksCube& RubiksCube3DArray::D() {

    // Rotate DOWN face clockwise
    rotateFaceClockwise(FACE::DOWN);

    COLOR temp[3];

    // Save FRONT bottom row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::FRONT)][2][i];
    }

    // LEFT -> FRONT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::FRONT)][2][i] =
            cube[static_cast<int>(FACE::LEFT)][2][i];
    }

    // BACK -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT)][2][i] =
            cube[static_cast<int>(FACE::BACK)][2][i];
    }

    // RIGHT -> BACK
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::BACK)][2][i] =
            cube[static_cast<int>(FACE::RIGHT)][2][i];
    }

    // FRONT -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT)][2][i] =
            temp[i];
    }

    return *this;
}

RubiksCube& RubiksCube3DArray::DPrime() {

    D();
    D();
    D();

    return *this;
}

RubiksCube& RubiksCube3DArray::D2() {

    D();
    D();

    return *this;
}

RubiksCube& RubiksCube3DArray::B() {

    // Rotate BACK face clockwise
    rotateFaceClockwise(FACE::BACK);

    COLOR temp[3];

    // Save UP top row
    for (int i = 0; i < 3; i++) {
        temp[i] =
            cube[static_cast<int>(FACE::UP)][0][i];
    }

    // RIGHT -> UP
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::UP)][0][i] =
            cube[static_cast<int>(FACE::RIGHT)][i][2];
    }

    // DOWN -> RIGHT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::RIGHT)][i][2] =
            cube[static_cast<int>(FACE::DOWN)][2][2 - i];
    }

    // LEFT -> DOWN
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::DOWN)][2][i] =
            cube[static_cast<int>(FACE::LEFT)][i][0];
    }

    // UP -> LEFT
    for (int i = 0; i < 3; i++) {
        cube[static_cast<int>(FACE::LEFT)][2 - i][0] =
            temp[i];
    }

    return *this;
}

RubiksCube& RubiksCube3DArray::BPrime() {

    B();
    B();
    B();

    return *this;
}

RubiksCube& RubiksCube3DArray::B2() {

    B();
    B();

    return *this;
}
