//
// Created by KIIT on 15-08-2026.
//

#ifndef CUBIQ_RUBIKSCUBE_H
#define CUBIQ_RUBIKSCUBE_H

#include <cstdint>
#include <string>

class RubiksCube {
public:
    enum class FACE {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        FRONT,
        BACK
    };

    enum class COLOR {
        RED,
        BLUE,
        GREEN,
        YELLOW,
        WHITE,
        ORANGE
    };

    enum class MOVE {
        F, FPRIME, F2,
        U, UPRIME, U2,
        L, LPRIME, L2,
        R, RPRIME, R2,
        D, DPRIME, D2,
        B, BPRIME, B2
    };

    virtual RubiksCube& F() = 0;
    virtual RubiksCube& FPrime() = 0;
    virtual RubiksCube& F2() = 0;

    virtual RubiksCube& U() = 0;
    virtual RubiksCube& UPrime() = 0;
    virtual RubiksCube& U2() = 0;

    virtual RubiksCube& L() = 0;
    virtual RubiksCube& LPrime() = 0;
    virtual RubiksCube& L2() = 0;

    virtual RubiksCube& R() = 0;
    virtual RubiksCube& RPrime() = 0;
    virtual RubiksCube& R2() = 0;

    virtual RubiksCube& D() = 0;
    virtual RubiksCube& DPrime() = 0;
    virtual RubiksCube& D2() = 0;

    virtual RubiksCube& B() = 0;
    virtual RubiksCube& BPrime() = 0;
    virtual RubiksCube& B2() = 0;

    virtual COLOR getColor(FACE face, int row, int col) const = 0;

    static char getColorLetter(COLOR color);

    void print() const;

    bool isSolved() const;

    RubiksCube& randomShuffle(int times);

    RubiksCube& move(MOVE move);

    // Corner positions use the order UFR, UFL, UBL, UBR,
    // DFR, DFL, DBR, DBL (indices 0 through 7).
    std::string getCornerColorString(std::uint8_t cornerPosition) const;
    std::uint8_t getCornerIndex(std::uint8_t cornerPosition) const;
    std::uint8_t getCornerOrientation(std::uint8_t cornerPosition) const;

    virtual ~RubiksCube() = default;
};


#endif //CUBIQ_RUBIKSCUBE_H
