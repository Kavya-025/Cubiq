#include "RubiksCube.h"

#include <stdexcept>
#include <iostream>
#include <random>
#include <string>

using namespace std;

char RubiksCube::getColorLetter(COLOR color) {
    switch (color) {
        case COLOR::RED:
            return 'R';
        case COLOR::GREEN:
            return 'G';
        case COLOR::BLUE:
            return 'B';
        case COLOR::YELLOW:
            return 'Y';
        case COLOR::ORANGE:
            return 'O';
        case COLOR::WHITE:
            return 'W';
    }
    return '?';
}

void RubiksCube::print() const {

    //PRINT UP
    for (int row = 0 ; row < 3 ; row++) {

        cout << "   ";

        for (int col = 0 ; col < 3 ; col++) {
            cout<<getColorLetter(getColor(FACE::UP, row, col)
                ) <<" ";
        }

        cout<< '\n';
    }
    cout<<'\n';


    //PRINT THE MID SECTION
    for (int row = 0; row < 3 ; row++) {
        FACE faces[] = {
            FACE::LEFT,
            FACE::FRONT,
            FACE::RIGHT,
            FACE::BACK
        };
        for (FACE face : faces) {
            for (int col = 0 ; col < 3 ; col++) {
                cout<<getColorLetter(getColor(face, row, col))<<" ";
            }
            cout<< " ";
        }
        cout<<'\n';
    }
    cout<<'\n';


    //DOWN SIDE PRINT
    for (int row = 0 ; row < 3 ; row++) {
        cout<<"    ";
        for (int col = 0 ; col < 3 ; col++) {
            cout<<getColorLetter(getColor(FACE::DOWN, row, col))<<" ";
        }
        cout<<'\n';
    }
}

bool RubiksCube::isSolved() const {

    // Check all 6 faces
    for (int face = 0; face < 6; face++) {

        FACE currentFace = static_cast<FACE>(face);

        // The center sticker determines the color
        // of this face.
        COLOR center = getColor(currentFace, 1, 1);

        // Check all 9 stickers on this face
        for (int row = 0; row < 3; row++) {

            for (int col = 0; col < 3; col++) {

                if (getColor(currentFace, row, col) != center) {
                    return false;
                }
            }
        }
    }

    return true;
}

RubiksCube& RubiksCube::move(MOVE move) {

    switch (move) {

        case MOVE::F:
            return F();

        case MOVE::FPRIME:
            return FPrime();

        case MOVE::F2:
            return F2();

        case MOVE::U:
            return U();

        case MOVE::UPRIME:
            return UPrime();

        case MOVE::U2:
            return U2();

        case MOVE::L:
            return L();

        case MOVE::LPRIME:
            return LPrime();

        case MOVE::L2:
            return L2();

        case MOVE::R:
            return R();

        case MOVE::RPRIME:
            return RPrime();

        case MOVE::R2:
            return R2();

        case MOVE::D:
            return D();

        case MOVE::DPRIME:
            return DPrime();

        case MOVE::D2:
            return D2();

        case MOVE::B:
            return B();

        case MOVE::BPRIME:
            return BPrime();

        case MOVE::B2:
            return B2();
    }

    // This should never be reached if MOVE contains
    // only the cases above.
    return *this;
}

RubiksCube& RubiksCube::randomShuffle(int times) {

    static random_device rd;
    static mt19937 generator(rd());

    uniform_int_distribution<int> distribution(0, 17);

    for (int i = 0; i < times; i++) {

        MOVE randomMove =
            static_cast<MOVE>(distribution(generator));

        move(randomMove);
    }

    return *this;
}

std::string RubiksCube::getCornerColorString(
    std::uint8_t cornerPosition
) const {
    using F = FACE;
    const auto makeColorString = [this](F firstFace, int firstRow, int firstCol,
                                        F secondFace, int secondRow, int secondCol,
                                        F thirdFace, int thirdRow, int thirdCol) {
        std::string colors;
        colors.reserve(3);
        colors.push_back(getColorLetter(getColor(firstFace, firstRow, firstCol)));
        colors.push_back(getColorLetter(getColor(secondFace, secondRow, secondCol)));
        colors.push_back(getColorLetter(getColor(thirdFace, thirdRow, thirdCol)));
        return colors;
    };

    switch (cornerPosition) {
        case 0: // UFR
            return makeColorString(F::UP, 2, 2, F::FRONT, 0, 2, F::RIGHT, 0, 0);
        case 1: // UFL
            return makeColorString(F::UP, 2, 0, F::FRONT, 0, 0, F::LEFT, 0, 2);
        case 2: // UBL
            return makeColorString(F::UP, 0, 0, F::BACK, 0, 2, F::LEFT, 0, 0);
        case 3: // UBR
            return makeColorString(F::UP, 0, 2, F::BACK, 0, 0, F::RIGHT, 0, 2);
        case 4: // DFR
            return makeColorString(F::DOWN, 0, 2, F::FRONT, 2, 2, F::RIGHT, 2, 0);
        case 5: // DFL
            return makeColorString(F::DOWN, 0, 0, F::FRONT, 2, 0, F::LEFT, 2, 2);
        case 6: // DBR
            return makeColorString(F::DOWN, 2, 2, F::BACK, 2, 0, F::RIGHT, 2, 2);
        case 7: // DBL
            return makeColorString(F::DOWN, 2, 0, F::BACK, 2, 2, F::LEFT, 2, 0);
        default:
            throw std::out_of_range("Corner position must be in [0, 7]");
    }
}

std::uint8_t RubiksCube::getCornerIndex(
    std::uint8_t cornerPosition
) const {
    const std::string colors = getCornerColorString(cornerPosition);
    std::uint8_t index = 0;

    // Bit 2 distinguishes the U layer (W) from the D layer (Y).
    if (colors.find('Y') != std::string::npos) {
        index |= 0b100;
    }
    // Bit 1 distinguishes the front (R) from the back (O).
    if (colors.find('O') != std::string::npos) {
        index |= 0b010;
    }
    // Bit 0 distinguishes the right (B) from the left (G).
    if (colors.find('G') != std::string::npos) {
        index |= 0b001;
    }

    return index;
}

std::uint8_t RubiksCube::getCornerOrientation(
    std::uint8_t cornerPosition
) const {
    const std::string colors = getCornerColorString(cornerPosition);
    std::size_t udStickerPosition = colors.find_first_of("WY");
    if (udStickerPosition == std::string::npos) {
        throw std::invalid_argument(
            "Corner does not contain a white or yellow sticker");
    }

    // Corner facelets are ordered by position in getCornerColorString(). Their
    // handedness alternates by corner, so some positions count the two twisted
    // orientations in reverse. This convention keeps the legal twist sum 0 mod 3.
    constexpr bool reverseOrientation[8] = {
        true, false, true, false, false, true, true, false
    };
    const auto orientation = static_cast<std::uint8_t>(udStickerPosition);
    if (reverseOrientation[cornerPosition] && orientation != 0) {
        return static_cast<std::uint8_t>(3 - orientation);
    }
    return orientation;
}
