#include "PatternDatabase.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

PatternDatabase::PatternDatabase(std::size_t size, std::uint8_t initialValue)
    : database_(size, initialValue),
      numItems_(initialValue == 0x0F ? 0 : size) {}

bool PatternDatabase::setNumMoves(const RubiksCube& cube,
                                  std::uint8_t numMoves) {
    return setNumMoves(getDatabaseIndex(cube), numMoves);
}

bool PatternDatabase::setNumMoves(std::uint32_t index,
                                  std::uint8_t numMoves) {
    if (numMoves > 0x0E) {
        throw std::invalid_argument(
            "Pattern database move counts must be from 0 to 14");
    }

    const std::uint8_t previous = database_.get(index);
    if (previous <= numMoves) {
        return false;
    }

    if (previous == 0x0F) {
        ++numItems_;
    }
    database_.set(index, numMoves);
    return true;
}

std::uint8_t PatternDatabase::getNumMoves(const RubiksCube& cube) const {
    return getNumMoves(getDatabaseIndex(cube));
}

std::uint8_t PatternDatabase::getNumMoves(std::uint32_t index) const {
    return database_.get(index);
}

std::size_t PatternDatabase::getSize() const noexcept {
    return database_.size();
}

std::size_t PatternDatabase::getNumItems() const noexcept {
    return numItems_;
}

bool PatternDatabase::isFull() const noexcept {
    return numItems_ == database_.size();
}

void PatternDatabase::toFile(const std::string& filePath) const {
    std::ofstream output(filePath, std::ios::binary | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("Could not open pattern database for writing");
    }

    output.write(reinterpret_cast<const char*>(database_.data()),
                 static_cast<std::streamsize>(database_.storageSize()));
    if (!output) {
        throw std::runtime_error("Failed while writing pattern database");
    }
}

bool PatternDatabase::fromFile(const std::string& filePath) {
    std::ifstream input(filePath, std::ios::binary | std::ios::ate);
    if (!input) {
        return false;
    }

    const std::streampos end = input.tellg();
    if (end < 0 ||
        static_cast<std::streamoff>(end) !=
            static_cast<std::streamoff>(database_.storageSize())) {
        return false;
    }

    std::vector<std::uint8_t> bytes(database_.storageSize());
    input.seekg(0, std::ios::beg);
    if (!bytes.empty()) {
        input.read(reinterpret_cast<char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
        if (!input) {
            return false;
        }
    }

    std::copy(bytes.begin(), bytes.end(), database_.data());
    numItems_ = 0;
    for (std::size_t index = 0; index < database_.size(); ++index) {
        if (database_.get(index) != 0x0F) {
            ++numItems_;
        }
    }
    return true;
}

std::vector<std::uint8_t> PatternDatabase::inflate() const {
    std::vector<std::uint8_t> values;
    values.reserve(database_.size());
    database_.inflate(values);
    return values;
}

void PatternDatabase::reset(std::uint8_t initialValue) {
    database_.reset(initialValue);
    numItems_ = initialValue == 0x0F ? 0 : database_.size();
}
