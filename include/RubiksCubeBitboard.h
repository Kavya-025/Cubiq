#ifndef RUBIKSCUBEBITBOARD_H
#define RUBIKSCUBEBITBOARD_H

#include "RubiksCube.h"
#include <cstddef>
#include <cstdint>
#include <functional>

class RubiksCubeBitboard : public RubiksCube {

private:

    // One 64-bit integer for each face.
    // Each face contains 8 stickers.
    // Each sticker uses 8 bits.
    uint64_t bitboard[6];

    // Get the COLOR stored at one sticker index.
    COLOR getSticker(int face, int index) const;

    // Store a COLOR at one sticker index.
    void setSticker(int face, int index, COLOR color);

    // Rotate the stickers of one face clockwise.
    void rotateFaceClockwise(FACE face);

public:

    RubiksCubeBitboard();

    RubiksCubeBitboard& operator=(const RubiksCubeBitboard&) = default;
    bool operator==(const RubiksCubeBitboard& other) const;

    struct Hash {
        std::size_t operator()(const RubiksCubeBitboard& cube) const noexcept;
    };

    COLOR getColor(
        FACE face,
        int row,
        int col
    ) const override;

    // Same signature as RubiksCube::isSolved(). The base method is not
    // virtual, so this is intentionally not marked override.
    bool isSolved() const;

    // U
    RubiksCube& U() override;
    RubiksCube& UPrime() override;
    RubiksCube& U2() override;

    // L
    RubiksCube& L() override;
    RubiksCube& LPrime() override;
    RubiksCube& L2() override;

    // F
    RubiksCube& F() override;
    RubiksCube& FPrime() override;
    RubiksCube& F2() override;

    // R
    RubiksCube& R() override;
    RubiksCube& RPrime() override;
    RubiksCube& R2() override;

    // D
    RubiksCube& D() override;
    RubiksCube& DPrime() override;
    RubiksCube& D2() override;

    // B
    RubiksCube& B() override;
    RubiksCube& BPrime() override;
    RubiksCube& B2() override;
};

namespace std {
template<>
struct hash<RubiksCubeBitboard> {
    std::size_t operator()(const RubiksCubeBitboard& cube) const noexcept {
        return RubiksCubeBitboard::Hash{}(cube);
    }
};
}

#endif
