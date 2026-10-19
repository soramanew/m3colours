#include <bit>
#include <limits>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

namespace {

constexpr std::size_t k_maxExactPixels = std::size_t{ 1 } << 20u;

constexpr unsigned k_binBits = 5;
constexpr unsigned k_binShift = 8 - k_binBits;
constexpr std::size_t k_binCount = std::size_t{ 1 } << (3 * k_binBits);
constexpr std::uint8_t k_lowMask = (1u << k_binShift) - 1;

struct Bin {
    std::uint32_t count = 0;
    std::uint32_t red = 0;
    std::uint32_t green = 0;
    std::uint32_t blue = 0;
};

std::size_t binIndex(Argb pixel) {
    return (static_cast<std::size_t>(pixel.red() >> k_binShift) << (2 * k_binBits)) |
           (static_cast<std::size_t>(pixel.green() >> k_binShift) << k_binBits) |
           static_cast<std::size_t>(pixel.blue() >> k_binShift);
}

std::uint8_t binMean(std::size_t index, unsigned shift, std::uint32_t lowSum, std::uint32_t count) {
    const auto base = ((index >> shift) & ((1u << k_binBits) - 1)) << k_binShift;
    return static_cast<std::uint8_t>(base + (lowSum + count / 2) / count);
}

WeightedPixels exactPixels(std::span<const Argb> pixels, bool opaqueOnly) {
    constexpr auto k_empty = std::numeric_limits<std::uint32_t>::max();
    constexpr auto k_fibonacci = 0x9E3779B97F4A7C15ull;

    const auto capacity = std::bit_ceil(std::max<std::size_t>(16, pixels.size() * 2));
    const auto shift = static_cast<unsigned>(64 - std::countr_zero(capacity));
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

WeightedPixels binnedPixels(std::span<const Argb> pixels, bool opaqueOnly) {
    std::vector<Bin> bins(k_binCount);
    for (const auto pixel : pixels) {
        if (opaqueOnly && !pixel.isOpaque())
            continue;

        auto& bin = bins[binIndex(pixel)];
        bin.count++;
        bin.red += static_cast<std::uint32_t>(pixel.red() & k_lowMask);
        bin.green += static_cast<std::uint32_t>(pixel.green() & k_lowMask);
        bin.blue += static_cast<std::uint32_t>(pixel.blue() & k_lowMask);
    }

    WeightedPixels result;
    for (std::size_t i = 0; i < k_binCount; i++) {
        const auto& bin = bins[i];
        if (bin.count == 0)
            continue;

        result.pixels.emplace_back(binMean(i, 2 * k_binBits, bin.red, bin.count),
            binMean(i, k_binBits, bin.green, bin.count), binMean(i, 0, bin.blue, bin.count));
        result.counts.push_back(bin.count);
    }
    return result;
}

} // namespace

WeightedPixels uniquePixels(std::span<const Argb> pixels, bool opaqueOnly) {
    if (pixels.size() > k_maxExactPixels)
        return binnedPixels(pixels, opaqueOnly);
    return exactPixels(pixels, opaqueOnly);
}

QuantiserResult celebi(const WeightedPixels& pixels, uint16_t maxColours) {
    if (maxColours == 0 || pixels.pixels.empty())
        return {};

    maxColours = std::min<uint16_t>(maxColours, 256);

    const auto wuResult = internal::wu(pixels, maxColours);
    return internal::wsMeans(pixels, wuResult, maxColours);
}

QuantiserResult celebi(std::span<const Argb> pixels, uint16_t maxColours, bool filterTransparent) {
    return celebi(uniquePixels(pixels, filterTransparent), maxColours);
}

} // namespace m3colours::quantise
