#ifndef CUBIQ_PATTERNDATABASE_H
#define CUBIQ_PATTERNDATABASE_H

#include "NibbleArray.h"
#include "RubiksCube.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Base storage and file operations shared by concrete pattern databases.
// A nibble value of 0xF denotes an entry that has not been populated.
class PatternDatabase {
private:
    NibbleArray database_;
    std::size_t numItems_;

public:
    explicit PatternDatabase(std::size_t size, std::uint8_t initialValue = 0x0F);
    virtual ~PatternDatabase() = default;

    virtual std::uint32_t getDatabaseIndex(const RubiksCube& cube) const = 0;

    bool setNumMoves(const RubiksCube& cube, std::uint8_t numMoves);
    bool setNumMoves(std::uint32_t index, std::uint8_t numMoves);
    std::uint8_t getNumMoves(const RubiksCube& cube) const;
    std::uint8_t getNumMoves(std::uint32_t index) const;

    std::size_t getSize() const noexcept;
    std::size_t getNumItems() const noexcept;
    bool isFull() const noexcept;

    void toFile(const std::string& filePath) const;
    bool fromFile(const std::string& filePath);
    std::vector<std::uint8_t> inflate() const;
    void reset(std::uint8_t initialValue = 0x0F);
};

#endif // CUBIQ_PATTERNDATABASE_H
