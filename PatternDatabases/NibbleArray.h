#ifndef CUBIQ_NIBBLEARRAY_H
#define CUBIQ_NIBBLEARRAY_H

#include <cstddef>
#include <cstdint>
#include <vector>

// Stores two 4-bit values in each byte. Even indices use the high nibble;
// odd indices use the low nibble.
class NibbleArray {
private:
    std::size_t size_;
    std::vector<std::uint8_t> bytes_;

    static std::uint8_t packedByte(std::uint8_t value);
    void checkPosition(std::size_t position) const;

public:
    explicit NibbleArray(std::size_t size, std::uint8_t initialValue = 0x0F);

    std::size_t size() const noexcept;
    std::uint8_t get(std::size_t position) const;
    void set(std::size_t position, std::uint8_t value);

    std::uint8_t* data() noexcept;
    const std::uint8_t* data() const noexcept;
    std::size_t storageSize() const noexcept;

    void inflate(std::vector<std::uint8_t>& destination) const;
    void reset(std::uint8_t value = 0x0F);
};

#endif // CUBIQ_NIBBLEARRAY_H
