#include <gtest/gtest.h>

#include "m3colours/contrast.hpp"

namespace m3colours::contrast {

namespace {

TEST(Contrast, RatioOfTonesOutOfBoundsInput) {
    EXPECT_NEAR(ratioOfTones(-10.0, 110.0), 21.0, 0.001);
}

TEST(Contrast, LighterImpossibleRatioErrors) {
    EXPECT_NEAR(lighter(90.0, 10.0), -1.0, 0.001);
}

TEST(Contrast, LighterOutOfBoundsInputAboveErrors) {
    EXPECT_NEAR(lighter(110.0, 2.0), -1.0, 0.001);
}

TEST(Contrast, LighterOutOfBoundsInputBelowErrors) {
    EXPECT_NEAR(lighter(-10.0, 2.0), -1.0, 0.001);
}

TEST(Contrast, LighterUnsafeReturnsMaxTone) {
    EXPECT_NEAR(lighterUnsafe(100.0, 2.0), 100.0, 0.001);
}

TEST(Contrast, DarkerImpossibleRatioErrors) {
    EXPECT_NEAR(darker(10.0, 20.0), -1.0, 0.001);
}

TEST(Contrast, DarkerOutOfBoundsInputAboveErrors) {
    EXPECT_NEAR(darker(110.0, 2.0), -1.0, 0.001);
}

TEST(Contrast, DarkerOutOfBoundsInputBelowErrors) {
    EXPECT_NEAR(darker(-10.0, 2.0), -1.0, 0.001);
}

TEST(Contrast, DarkerUnsafeReturnsMinTone) {
    EXPECT_NEAR(darkerUnsafe(0.0, 2.0), 0.0, 0.001);
}

TEST(Contrast, LighterAchievesRatio) {
    const auto tone = lighter(20.0, 4.5);
    EXPECT_GT(tone, 20.0);
    EXPECT_GE(ratioOfTones(tone, 20.0), 4.5);
}

TEST(Contrast, DarkerAchievesRatio) {
    const auto tone = darker(80.0, 4.5);
    EXPECT_LT(tone, 80.0);
    EXPECT_GE(ratioOfTones(tone, 80.0), 4.5);
}

} // namespace

} // namespace m3colours::contrast
