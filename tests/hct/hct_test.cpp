#include <gtest/gtest.h>

#include <cmath>

#include "m3colours/hct.hpp"

namespace m3colours::hct {

namespace {

constexpr Argb k_red{ 0xffff0000 };
constexpr Argb k_green{ 0xff00ff00 };
constexpr Argb k_blue{ 0xff0000ff };
constexpr Argb k_white{ 0xffffffff };
constexpr Argb k_black{ 0xff000000 };

void expectCam(Argb argb, double hue, double chroma, double j, double m, double s, double q) {
    const auto cam = Cam16::fromArgb(argb);
    EXPECT_NEAR(cam.hue, hue, 0.001);
    EXPECT_NEAR(cam.chroma, chroma, 0.001);
    EXPECT_NEAR(cam.j, j, 0.001);
    EXPECT_NEAR(cam.m, m, 0.001);
    EXPECT_NEAR(cam.s, s, 0.001);
    EXPECT_NEAR(cam.q, q, 0.001);
}

TEST(Cam16, Red) {
    expectCam(k_red, 27.408, 113.358, 46.445, 89.494, 91.890, 105.989);
}

TEST(Cam16, Green) {
    expectCam(k_green, 142.140, 108.410, 79.332, 85.588, 78.605, 138.520);
}

TEST(Cam16, Blue) {
    expectCam(k_blue, 282.788, 87.231, 25.466, 68.867, 93.675, 78.481);
}

TEST(Cam16, White) {
    expectCam(k_white, 209.492, 2.869, 100.0, 2.265, 12.068, 155.521);
}

TEST(Cam16, Black) {
    expectCam(k_black, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

TEST(Cam16, RoundTripRed) {
    EXPECT_EQ(Cam16::fromArgb(k_red).toArgb(), k_red);
}

TEST(Cam16, RoundTripGreen) {
    EXPECT_EQ(Cam16::fromArgb(k_green).toArgb(), k_green);
}

TEST(Cam16, RoundTripBlue) {
    EXPECT_EQ(Cam16::fromArgb(k_blue).toArgb(), k_blue);
}

TEST(Hct, Green) {
    const Hct hct(k_green);
    EXPECT_NEAR(hct.hue(), 142.139, 0.01);
    EXPECT_NEAR(hct.chroma(), 108.410, 0.01);
    EXPECT_NEAR(hct.tone(), 87.737, 0.01);
}

TEST(Hct, Blue) {
    const Hct hct(k_blue);
    EXPECT_NEAR(hct.hue(), 282.788, 0.01);
    EXPECT_NEAR(hct.chroma(), 87.230, 0.01);
    EXPECT_NEAR(hct.tone(), 32.302, 0.01);
}

TEST(Hct, BlueTone90) {
    const Hct hct(282.788, 87.230, 90.0);
    EXPECT_NEAR(hct.hue(), 282.239, 0.01);
    EXPECT_NEAR(hct.chroma(), 19.144, 0.01);
    EXPECT_NEAR(hct.tone(), 90.035, 0.01);
}

TEST(ViewingConditions, Default) {
    const auto& vc = k_defaultViewingConditions;
    EXPECT_NEAR(vc.n, 0.184, 0.001);
    EXPECT_NEAR(vc.aw, 29.981, 0.001);
    EXPECT_NEAR(vc.nbb, 1.017, 0.001);
    EXPECT_NEAR(vc.ncb, 1.017, 0.001);
    EXPECT_NEAR(vc.c, 0.69, 0.001);
    EXPECT_NEAR(vc.nc, 1.0, 0.001);
    EXPECT_NEAR(vc.rgbD[0], 1.021, 0.001);
    EXPECT_NEAR(vc.rgbD[1], 0.986, 0.001);
    EXPECT_NEAR(vc.rgbD[2], 0.934, 0.001);
    EXPECT_NEAR(vc.fl, 0.388, 0.001);
    EXPECT_NEAR(vc.flRoot, 0.789, 0.001);
    EXPECT_NEAR(vc.z, 1.909, 0.001);
}

bool colourIsOnBoundary(Argb argb) {
    return argb.red() == 0 || argb.red() == 255 || argb.green() == 0 || argb.green() == 255 || argb.blue() == 0 ||
           argb.blue() == 255;
}

TEST(Solver, ReturnsSufficientlyCloseColour) {
    for (int hue = 15; hue < 360; hue += 30) {
        for (int chroma = 0; chroma <= 100; chroma += 10) {
            for (int tone = 20; tone <= 80; tone += 10) {
                const Hct hct(hue, chroma, tone);

                if (chroma > 0)
                    EXPECT_LE(std::abs(hct.hue() - hue), 4.0);

                EXPECT_GE(hct.chroma(), 0.0);
                EXPECT_LE(hct.chroma(), chroma + 2.5);

                if (hct.chroma() < chroma - 2.5)
                    EXPECT_TRUE(colourIsOnBoundary(hct.toArgb()));

                EXPECT_LE(std::abs(hct.tone() - tone), 0.5);
            }
        }
    }
}

} // namespace

} // namespace m3colours::hct
