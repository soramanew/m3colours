#include <gtest/gtest.h>

#include "m3colours/scheme.hpp"

namespace m3colours::scheme {

namespace {

TEST(SchemeAndroid, BlueLight) {
    EXPECT_EQ(SchemeAndroid::light(Argb{ 0xff0000ff }).colourAccentPrimary, Argb{ 0xffe0e0ff });
}

TEST(SchemeAndroid, BlueDark) {
    EXPECT_EQ(SchemeAndroid::dark(Argb{ 0xff0000ff }).colourAccentPrimary, Argb{ 0xffe0e0ff });
}

TEST(SchemeAndroid, ThirdPartyLight) {
    const auto scheme = SchemeAndroid::light(Argb{ 0xff6750a4 });
    EXPECT_EQ(scheme.colourAccentPrimary, Argb{ 0xffe9ddff });
    EXPECT_EQ(scheme.colourAccentSecondary, Argb{ 0xffe8def8 });
    EXPECT_EQ(scheme.colourAccentTertiary, Argb{ 0xffffd9e3 });
    EXPECT_EQ(scheme.colourSurface, Argb{ 0xfffdf8fd });
    EXPECT_EQ(scheme.textColourPrimary, Argb{ 0xff1c1b1e });
}

TEST(SchemeAndroid, ThirdPartyDark) {
    const auto scheme = SchemeAndroid::dark(Argb{ 0xff6750a4 });
    EXPECT_EQ(scheme.colourAccentPrimary, Argb{ 0xffe9ddff });
    EXPECT_EQ(scheme.colourAccentSecondary, Argb{ 0xffe8def8 });
    EXPECT_EQ(scheme.colourAccentTertiary, Argb{ 0xffffd9e3 });
    EXPECT_EQ(scheme.colourSurface, Argb{ 0xff313033 });
    EXPECT_EQ(scheme.textColourPrimary, Argb{ 0xfff4eff4 });
}

} // namespace

} // namespace m3colours::scheme
