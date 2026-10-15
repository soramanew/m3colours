#include <gtest/gtest.h>

#include "m3colours/blend.hpp"

namespace m3colours::blend {

namespace {

constexpr Argb k_red{ 0xffff0000 };
constexpr Argb k_blue{ 0xff0000ff };
constexpr Argb k_green{ 0xff00ff00 };
constexpr Argb k_yellow{ 0xffffff00 };

TEST(Harmonise, RedToBlue) {
    EXPECT_EQ(harmonise(k_red, k_blue), Argb{ 0xffFB0057 });
}

TEST(Harmonise, RedToGreen) {
    EXPECT_EQ(harmonise(k_red, k_green), Argb{ 0xffD85600 });
}

TEST(Harmonise, RedToYellow) {
    EXPECT_EQ(harmonise(k_red, k_yellow), Argb{ 0xffD85600 });
}

TEST(Harmonise, BlueToGreen) {
    EXPECT_EQ(harmonise(k_blue, k_green), Argb{ 0xff0047A3 });
}

TEST(Harmonise, BlueToRed) {
    EXPECT_EQ(harmonise(k_blue, k_red), Argb{ 0xff5700DC });
}

TEST(Harmonise, BlueToYellow) {
    EXPECT_EQ(harmonise(k_blue, k_yellow), Argb{ 0xff0047A3 });
}

TEST(Harmonise, GreenToBlue) {
    EXPECT_EQ(harmonise(k_green, k_blue), Argb{ 0xff00FC94 });
}

TEST(Harmonise, GreenToRed) {
    EXPECT_EQ(harmonise(k_green, k_red), Argb{ 0xffB1F000 });
}

TEST(Harmonise, GreenToYellow) {
    EXPECT_EQ(harmonise(k_green, k_yellow), Argb{ 0xffB1F000 });
}

TEST(Harmonise, YellowToBlue) {
    EXPECT_EQ(harmonise(k_yellow, k_blue), Argb{ 0xffEBFFBA });
}

TEST(Harmonise, YellowToGreen) {
    EXPECT_EQ(harmonise(k_yellow, k_green), Argb{ 0xffEBFFBA });
}

TEST(Harmonise, YellowToRed) {
    EXPECT_EQ(harmonise(k_yellow, k_red), Argb{ 0xffFFF6E3 });
}

} // namespace

} // namespace m3colours::blend
