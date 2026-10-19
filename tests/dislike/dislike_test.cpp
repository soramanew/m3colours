#include <gtest/gtest.h>

#include "m3colours/dislike.hpp"

namespace m3colours::dislike {

namespace {

constexpr Argb k_bileColours[]{
    Argb{ 0xff95884B },
    Argb{ 0xff716B40 },
    Argb{ 0xffB08E00 },
    Argb{ 0xff4C4308 },
    Argb{ 0xff464521 },
};

TEST(Dislike, LikesMonkSkinToneScaleColours) {
    constexpr Argb k_colours[]{
        Argb{ 0xfff6ede4 },
        Argb{ 0xfff3e7db },
        Argb{ 0xfff7ead0 },
        Argb{ 0xffeadaba },
        Argb{ 0xffd7bd96 },
        Argb{ 0xffa07e56 },
        Argb{ 0xff825c43 },
        Argb{ 0xff604134 },
        Argb{ 0xff3a312a },
        Argb{ 0xff292420 },
    };
    for (const auto colour : k_colours) {
        EXPECT_FALSE(isDisliked(Hct{ colour }));
    }
}

TEST(Dislike, DislikesBileColours) {
    for (const auto colour : k_bileColours) {
        EXPECT_TRUE(isDisliked(Hct{ colour }));
    }
}

TEST(Dislike, MakesBileColoursLikable) {
    for (const auto colour : k_bileColours) {
        const Hct hct{ colour };
        EXPECT_TRUE(isDisliked(hct));
        EXPECT_FALSE(isDisliked(fixIfDisliked(hct)));
    }
}

TEST(Dislike, LikesTone67Colours) {
    const Hct colour{ 100.0, 50.0, 67.0 };
    EXPECT_FALSE(isDisliked(colour));
    EXPECT_EQ(fixIfDisliked(colour).toArgb(), colour.toArgb());
}

} // namespace

} // namespace m3colours::dislike
