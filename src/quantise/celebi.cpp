#include <bit>
#include <limits>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

namespace internal {

WeightedPixels uniquePixels(const std::vector<Argb>& pixels, bool opaqueOnly) {
    constexpr auto k_empty = std::numeric_limits<std::uint32_t>::max();
    constexpr auto k_fibonacci = 0x9E3779B97F4A7C15ull;

    const auto capacity = std::bit_ceil(std::max<std::size_t>(16, pixels.size() * 2));
    const auto shift = 64 - std::countr_zero(capacity);
    std::vector<std::uint32_t> slots(capacity, k_empty);

    WeightedPixels result;
    for (const auto pixel : pixels) {
        if (opaqueOnly && !pixel.isOpaque())
            continue;

        auto slot = static_cast<std::size_t>((pixel.value() * k_fibonacci) >> shift);
        while (slots[slot] != k_empty && result.pixels[slots[slot]] != pixel)
            slot = (slot + 1) & (capacity - 1);

        if (slots[slot] == k_empty) {
            slots[slot] = static_cast<std::uint32_t>(result.pixels.size());
            result.pixels.push_back(pixel);
            result.counts.push_back(1);
        } else {
            result.counts[slots[slot]]++;
        }
    }
    return result;
}

} // namespace internal

QuantiserResult celebi(const std::vector<Argb>& pixels, uint16_t maxColours, bool filterTransparent) {
    if (maxColours == 0 || pixels.empty())
        return {};

    maxColours = std::min<uint16_t>(maxColours, 256);

    const auto unique = internal::uniquePixels(pixels, filterTransparent);
    const auto wuResult = internal::wu(unique, maxColours);
    return internal::wsMeans(unique, wuResult, maxColours);
}

} // namespace m3colours::quantise
