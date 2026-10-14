#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

namespace {

std::vector<Argb> repeated(std::uint32_t colour, std::size_t count) {
    return std::vector<Argb>(count, Argb{ colour });
}

TEST(Wsmeans, FullImage) {
    std::vector<Argb> pixels;
    pixels.reserve(12544);
    for (std::uint32_t i = 0; i < 12544; i++)
        pixels.emplace_back(i % 8000);

    wsMeans(pixels, {}, 128);
}

TEST(Wsmeans, OneRedAndO) {
    auto result = wsMeans(repeated(0xff141216, 1), {}, 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff141216 }], 1u);
}

TEST(Wsmeans, OneRed) {
    auto result = wsMeans(repeated(0xffff0000, 1), {}, 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xffff0000 }], 1u);
}

TEST(Wsmeans, OneGreen) {
    auto result = wsMeans(repeated(0xff00ff00, 1), {}, 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff00ff00 }], 1u);
}

TEST(Wsmeans, OneBlue) {
    auto result = wsMeans(repeated(0xff0000ff, 1), {}, 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff0000ff }], 1u);
}

TEST(Wsmeans, FiveBlue) {
    auto result = wsMeans(repeated(0xff0000ff, 5), {}, 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff0000ff }], 5u);
}

} // namespace

} // namespace m3colours::quantise
