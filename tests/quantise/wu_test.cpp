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

TEST(Wu, FullImage) {
    std::vector<Argb> pixels;
    pixels.reserve(12544);
    for (std::uint32_t i = 0; i < 12544; i++)
        pixels.emplace_back(i % 8000);

    wu(pixels, 128);
}

TEST(Wu, TwoRedThreeGreen) {
    const std::vector<Argb> pixels{
        Argb{ 0xffff0000 },
        Argb{ 0xffff0000 },
        Argb{ 0xffff0000 },
        Argb{ 0xff00ff00 },
        Argb{ 0xff00ff00 },
    };
    EXPECT_EQ(wu(pixels, 256).size(), 2u);
}

TEST(Wu, OneRed) {
    EXPECT_EQ(wu(repeated(0xffff0000, 1), 256), std::vector<Argb>{ Argb{ 0xffff0000 } });
}

TEST(Wu, OneGreen) {
    EXPECT_EQ(wu(repeated(0xff00ff00, 1), 256), std::vector<Argb>{ Argb{ 0xff00ff00 } });
}

TEST(Wu, OneBlue) {
    EXPECT_EQ(wu(repeated(0xff0000ff, 1), 256), std::vector<Argb>{ Argb{ 0xff0000ff } });
}

TEST(Wu, FiveBlue) {
    EXPECT_EQ(wu(repeated(0xff0000ff, 5), 256), std::vector<Argb>{ Argb{ 0xff0000ff } });
}

TEST(Wu, OneRedAndO) {
    EXPECT_EQ(wu(repeated(0xff141216, 1), 256), std::vector<Argb>{ Argb{ 0xff141216 } });
}

TEST(Wu, RedGreenBlue) {
    const std::vector<Argb> pixels{ Argb{ 0xffff0000 }, Argb{ 0xff00ff00 }, Argb{ 0xff0000ff } };
    const std::vector<Argb> expected{ Argb{ 0xff0000ff }, Argb{ 0xffff0000 }, Argb{ 0xff00ff00 } };
    EXPECT_EQ(wu(pixels, 256), expected);
}

TEST(Wu, MixedColours) {
    const std::vector<Argb> pixels{
        Argb{ 0xff010203 },
        Argb{ 0xff665544 },
        Argb{ 0xff708090 },
        Argb{ 0xffc0ffee },
        Argb{ 0xfffedcba },
    };
    EXPECT_EQ(wu(pixels, 256).size(), 5u);
}

} // namespace

} // namespace m3colours::quantise
