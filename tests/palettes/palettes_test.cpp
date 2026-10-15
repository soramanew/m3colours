#include <gtest/gtest.h>

#include <cmath>

#include "m3colours/palettes.hpp"

namespace m3colours::palettes {

namespace {

constexpr Argb k_blue{ 0xff0000ff };

void expectBlueTones(const TonalPalette& palette) {
    EXPECT_EQ(palette.tone(100), Argb{ 0xffffffff });
    EXPECT_EQ(palette.tone(95), Argb{ 0xfff1efff });
    EXPECT_EQ(palette.tone(90), Argb{ 0xffe0e0ff });
    EXPECT_EQ(palette.tone(80), Argb{ 0xffbec2ff });
    EXPECT_EQ(palette.tone(70), Argb{ 0xff9da3ff });
    EXPECT_EQ(palette.tone(60), Argb{ 0xff7c84ff });
    EXPECT_EQ(palette.tone(50), Argb{ 0xff5a64ff });
    EXPECT_EQ(palette.tone(40), Argb{ 0xff343dff });
    EXPECT_EQ(palette.tone(30), Argb{ 0xff0000ef });
    EXPECT_EQ(palette.tone(20), Argb{ 0xff0001ac });
    EXPECT_EQ(palette.tone(10), Argb{ 0xff00006e });
    EXPECT_EQ(palette.tone(0), Argb{ 0xff000000 });
}

TEST(TonalPalette, OfBlue) {
    expectBlueTones(TonalPalette::fromArgb(k_blue));
}

TEST(CorePalette, OfBlue) {
    const auto core = CorePalette::of(k_blue);
    expectBlueTones(core.a1);

    EXPECT_EQ(core.a2.tone(100), Argb{ 0xffffffff });
    EXPECT_EQ(core.a2.tone(95), Argb{ 0xfff1efff });
    EXPECT_EQ(core.a2.tone(90), Argb{ 0xffe1e0f9 });
    EXPECT_EQ(core.a2.tone(80), Argb{ 0xffc5c4dd });
    EXPECT_EQ(core.a2.tone(70), Argb{ 0xffa9a9c1 });
    EXPECT_EQ(core.a2.tone(60), Argb{ 0xff8f8fa6 });
    EXPECT_EQ(core.a2.tone(50), Argb{ 0xff75758b });
    EXPECT_EQ(core.a2.tone(40), Argb{ 0xff5c5d72 });
    EXPECT_EQ(core.a2.tone(30), Argb{ 0xff444559 });
    EXPECT_EQ(core.a2.tone(20), Argb{ 0xff2e2f42 });
    EXPECT_EQ(core.a2.tone(10), Argb{ 0xff191a2c });
    EXPECT_EQ(core.a2.tone(0), Argb{ 0xff000000 });
}

TEST(CorePalette, ContentOfBlue) {
    const auto core = CorePalette::contentOf(k_blue);
    expectBlueTones(core.a1);

    EXPECT_EQ(core.a2.tone(100), Argb{ 0xffffffff });
    EXPECT_EQ(core.a2.tone(95), Argb{ 0xfff1efff });
    EXPECT_EQ(core.a2.tone(90), Argb{ 0xffe0e0ff });
    EXPECT_EQ(core.a2.tone(80), Argb{ 0xffc1c3f4 });
    EXPECT_EQ(core.a2.tone(70), Argb{ 0xffa5a7d7 });
    EXPECT_EQ(core.a2.tone(60), Argb{ 0xff8b8dbb });
    EXPECT_EQ(core.a2.tone(50), Argb{ 0xff7173a0 });
    EXPECT_EQ(core.a2.tone(40), Argb{ 0xff585b86 });
    EXPECT_EQ(core.a2.tone(30), Argb{ 0xff40436d });
    EXPECT_EQ(core.a2.tone(20), Argb{ 0xff2a2d55 });
    EXPECT_EQ(core.a2.tone(10), Argb{ 0xff14173f });
    EXPECT_EQ(core.a2.tone(0), Argb{ 0xff000000 });
}

TEST(KeyColour, ExactChroma) {
    const auto& result = TonalPalette::fromHueAndChroma(50.0, 60.0).keyColour();
    EXPECT_LT(std::abs(result.hue() - 50.0), 10.0);
    EXPECT_LT(std::abs(result.chroma() - 60.0), 0.5);
    EXPECT_GT(result.tone(), 0.0);
    EXPECT_LT(result.tone(), 100.0);
}

TEST(KeyColour, UnusuallyHighChroma) {
    const auto& result = TonalPalette::fromHueAndChroma(149.0, 200.0).keyColour();
    EXPECT_LT(std::abs(result.hue() - 149.0), 10.0);
    EXPECT_GT(result.chroma(), 89.0);
    EXPECT_GT(result.tone(), 0.0);
    EXPECT_LT(result.tone(), 100.0);
}

TEST(KeyColour, UnusuallyLowChroma) {
    const auto& result = TonalPalette::fromHueAndChroma(50.0, 3.0).keyColour();
    EXPECT_LT(std::abs(result.hue() - 50.0), 10.0);
    EXPECT_LT(std::abs(result.chroma() - 3.0), 0.5);
    EXPECT_LT(std::abs(result.tone() - 50.0), 0.5);
}

} // namespace

} // namespace m3colours::palettes
