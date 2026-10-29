#include "NibbleArray.h"

#include <algorithm>
#include <stdexcept>

std::uint8_t NibbleArray::packedByte(std::uint8_t value) {
    if (value > 0x0F) {
        throw std::invalid_argument("NibbleArray values must fit in 4 bits");
    }
    return static_cast<std::uint8_t>((value << 4) | value);
}

void NibbleArray::checkPosition(std::size_t position) const {
    if (position >= size_) {
        throw std::out_of_range("NibbleArray index is out of range");
    }
}

NibbleArray::NibbleArray(std::size_t size, std::uint8_t initialValue)
    : size_(size),
      bytes_(size / 2 + size % 2, packedByte(initialValue)) {}

std::size_t NibbleArray::size() const noexcept {
    return size_;
}

std::uint8_t NibbleArray::get(std::size_t position) const {
    checkPosition(position);
    const std::uint8_t packed = bytes_[position / 2];
    return position % 2 == 0
        ? static_cast<std::uint8_t>((packed >> 4) & 0x0F)
        : static_cast<std::uint8_t>(packed & 0x0F);
}

void NibbleArray::set(std::size_t position, std::uint8_t value) {
    checkPosition(position);
    if (value > 0x0F) {
        throw std::invalid_argument("NibbleArray values must fit in 4 bits");
    }

    std::uint8_t& packed = bytes_[position / 2];
    if (position % 2 == 0) {
        packed = static_cast<std::uint8_t>((packed & 0x0F) | (value << 4));
    } else {
        packed = static_cast<std::uint8_t>((packed & 0xF0) | value);
    }
}

std::uint8_t* NibbleArray::data() noexcept {
    return bytes_.data();
}

const std::uint8_t* NibbleArray::data() const noexcept {
    return bytes_.data();
}

std::size_t NibbleArray::storageSize() const noexcept {
    return bytes_.size();
}

void NibbleArray::inflate(std::vector<std::uint8_t>& destination) const {
    destination.reserve(destination.size() + size_);
    for (std::size_t index = 0; index < size_; ++index) {
        destination.push_back(get(index));
    }
}

void NibbleArray::reset(std::uint8_t value) {
    const std::uint8_t packed = packedByte(value);
    std::fill(bytes_.begin(), bytes_.end(), packed);
}
