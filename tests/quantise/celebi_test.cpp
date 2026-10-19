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

TEST(Celebi, FullImage) {
    std::vector<Argb> pixels;
    pixels.reserve(12544);
    for (std::uint32_t i = 0; i < 12544; i++)
        pixels.emplace_back(i % 8000);

    celebi(pixels, 128);
}

TEST(Celebi, WeightedPixelsMatchesSpan) {
    std::vector<Argb> pixels;
    pixels.reserve(12544);
    for (std::uint32_t i = 0; i < 12544; i++)
        pixels.emplace_back(0xff000000 | (i % 8000));

    const auto unique = uniquePixels(pixels, true);
    EXPECT_EQ(unique.pixels.size(), 8000u);
    EXPECT_EQ(celebi(unique, 128).colourToCount, celebi(pixels, 128).colourToCount);
}

TEST(Celebi, EmptyWeightedPixels) {
    EXPECT_TRUE(celebi(WeightedPixels{}, 128).colourToCount.empty());
}

TEST(Celebi, OneRed) {
    auto result = celebi(repeated(0xffff0000, 1), 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xffff0000 }], 1u);
}

TEST(Celebi, OneGreen) {
    auto result = celebi(repeated(0xff00ff00, 1), 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff00ff00 }], 1u);
}

TEST(Celebi, OneBlue) {
    auto result = celebi(repeated(0xff0000ff, 1), 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff0000ff }], 1u);
}

TEST(Celebi, FiveBlue) {
    auto result = celebi(repeated(0xff0000ff, 5), 256);
    EXPECT_EQ(result.colourToCount.size(), 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff0000ff }], 5u);
}

TEST(Celebi, OneRedOneGreenOneBlue) {
    std::vector<Argb> pixels{ Argb{ 0xffff0000 }, Argb{ 0xff00ff00 }, Argb{ 0xff0000ff } };
    auto result = celebi(pixels, 256);
    EXPECT_EQ(result.colourToCount.size(), 3u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xffff0000 }], 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff00ff00 }], 1u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff0000ff }], 1u);
}

TEST(Celebi, TwoRedThreeGreen) {
    std::vector<Argb> pixels{
        Argb{ 0xffff0000 },
        Argb{ 0xffff0000 },
        Argb{ 0xff00ff00 },
        Argb{ 0xff00ff00 },
        Argb{ 0xff00ff00 },
    };
    auto result = celebi(pixels, 256);
    EXPECT_EQ(result.colourToCount.size(), 2u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xffff0000 }], 2u);
    EXPECT_EQ(result.colourToCount[Argb{ 0xff00ff00 }], 3u);
}

TEST(Celebi, NoColours) {
    const auto result = celebi(repeated(0xffffffff, 1), 0);
    EXPECT_TRUE(result.colourToCount.empty());
    EXPECT_TRUE(result.inputToCluster.empty());
}

TEST(Celebi, SingleTransparent) {
    const auto result = celebi(repeated(0x20f93013, 1), 1);
    EXPECT_TRUE(result.colourToCount.empty());
    EXPECT_TRUE(result.inputToCluster.empty());
}

TEST(Celebi, TooManyColours) {
    const auto result = celebi(std::vector<Argb>{}, 32767);
    EXPECT_TRUE(result.colourToCount.empty());
    EXPECT_TRUE(result.inputToCluster.empty());
}

} // namespace

} // namespace m3colours::quantise
