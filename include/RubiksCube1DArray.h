#ifndef RUBIKSCUBE1DARRAY_H
#define RUBIKSCUBE1DARRAY_H

#include "RubiksCube.h"
#include <cstddef>
#include <functional>

class RubiksCube1DArray : public RubiksCube {

private:
    COLOR cube[54];

    void rotateFaceClockwise(FACE face);

public:
    RubiksCube1DArray();

    RubiksCube1DArray& operator=(const RubiksCube1DArray&) = default;
    bool operator==(const RubiksCube1DArray& other) const;

    struct Hash {
        std::size_t operator()(const RubiksCube1DArray& cube) const noexcept;
    };

    COLOR getColor(FACE face, int row, int col) const override;

    virtual bool isSolved();

    RubiksCube& F() override;
    RubiksCube& FPrime() override;
    RubiksCube& F2() override;

    RubiksCube& U() override;
    RubiksCube& UPrime() override;
    RubiksCube& U2() override;

    RubiksCube& L() override;
    RubiksCube& LPrime() override;
    RubiksCube& L2() override;

    RubiksCube& R() override;
    RubiksCube& RPrime() override;
    RubiksCube& R2() override;

    RubiksCube& D() override;
    RubiksCube& DPrime() override;
    RubiksCube& D2() override;

    RubiksCube& B() override;
    RubiksCube& BPrime() override;
    RubiksCube& B2() override;
};

namespace std {
template<>
struct hash<RubiksCube1DArray> {
    std::size_t operator()(const RubiksCube1DArray& cube) const noexcept {
        return RubiksCube1DArray::Hash{}(cube);
    }
};
}

#endif
