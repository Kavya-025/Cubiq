#ifndef CUBIQ_RUBIKSCUBE3DARRAY_H
#define CUBIQ_RUBIKSCUBE3DARRAY_H

#include "RubiksCube.h"
#include <cstddef>
#include <functional>

class RubiksCube3DArray : public RubiksCube {

private:

    COLOR cube[6][3][3];

    void rotateFaceClockwise(FACE face);

public:

    RubiksCube3DArray();

    RubiksCube3DArray& operator=(const RubiksCube3DArray&) = default;
    bool operator==(const RubiksCube3DArray& other) const;

    struct Hash {
        std::size_t operator()(const RubiksCube3DArray& cube) const noexcept;
    };

    COLOR getColor(FACE face, int row, int col) const override;

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
struct hash<RubiksCube3DArray> {
    std::size_t operator()(const RubiksCube3DArray& cube) const noexcept {
        return RubiksCube3DArray::Hash{}(cube);
    }
};
}

#endif // CUBIQ_RUBIKSCUBE3DARRAY_H
