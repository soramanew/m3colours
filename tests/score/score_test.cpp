#include <gtest/gtest.h>

#include "m3colours/score.hpp"

namespace m3colours::score {

namespace {

using Population = std::map<Argb, std::uint32_t>;

void expectRanked(const std::vector<Argb>& ranked, std::initializer_list<std::uint32_t> expected) {
    ASSERT_EQ(ranked.size(), expected.size());
    auto it = expected.begin();
    for (const auto colour : ranked)
        EXPECT_EQ(colour, Argb{ *it++ });
}

TEST(Score, PrioritisesChroma) {
    const Population colours{ { Argb{ 0xff000000 }, 1 }, { Argb{ 0xffffffff }, 1 }, { Argb{ 0xff0000ff }, 1 } };
    expectRanked(score(colours, { .desired = 4 }), { 0xff0000ff });
}

TEST(Score, PrioritisesChromaWhenProportionsEqual) {
    const Population colours{ { Argb{ 0xffff0000 }, 1 }, { Argb{ 0xff00ff00 }, 1 }, { Argb{ 0xff0000ff }, 1 } };
    expectRanked(score(colours, { .desired = 4 }), { 0xffff0000, 0xff00ff00, 0xff0000ff });
}

TEST(Score, GeneratesGBlueWhenNoColoursAvailable) {
    const Population colours{ { Argb{ 0xff000000 }, 1 } };
    expectRanked(score(colours, { .desired = 4 }), { 0xff4285f4 });
}

TEST(Score, DedupesNearbyHues) {
    const Population colours{ { Argb{ 0xff008772 }, 1 }, { Argb{ 0xff318477 }, 1 } };
    expectRanked(score(colours, { .desired = 4 }), { 0xff008772 });
}

TEST(Score, MaximisesHueDistance) {
    const Population colours{ { Argb{ 0xff008772 }, 1 }, { Argb{ 0xff008587 }, 1 }, { Argb{ 0xff007ebc }, 1 } };
    expectRanked(score(colours, { .desired = 2 }), { 0xff007ebc, 0xff008772 });
}

TEST(Score, GeneratedScenarioOne) {
    const Population colours{ { Argb{ 0xff7ea16d }, 67 }, { Argb{ 0xffd8ccae }, 67 }, { Argb{ 0xff835c0d }, 49 } };
    expectRanked(score(colours, { .desired = 3, .fallbackColour = Argb{ 0xff8d3819 }, .filter = false }),
        { 0xff7ea16d, 0xffd8ccae, 0xff835c0d });
}

TEST(Score, GeneratedScenarioTwo) {
    const Population colours{ { Argb{ 0xffd33881 }, 14 }, { Argb{ 0xff3205cc }, 77 }, { Argb{ 0xff0b48cf }, 36 },
        { Argb{ 0xffa08f5d }, 81 } };
    expectRanked(score(colours, { .desired = 4, .fallbackColour = Argb{ 0xff7d772b }, .filter = true }),
        { 0xff3205cc, 0xffa08f5d, 0xffd33881 });
}

TEST(Score, GeneratedScenarioThree) {
    const Population colours{ { Argb{ 0xffbe94a6 }, 23 }, { Argb{ 0xffc33fd7 }, 42 }, { Argb{ 0xff899f36 }, 90 },
        { Argb{ 0xff94c574 }, 82 } };
    expectRanked(score(colours, { .desired = 3, .fallbackColour = Argb{ 0xffaa79a4 }, .filter = true }),
        { 0xff94c574, 0xffc33fd7, 0xffbe94a6 });
}

TEST(Score, GeneratedScenarioFour) {
    const Population colours{ { Argb{ 0xffdf241c }, 85 }, { Argb{ 0xff685859 }, 44 }, { Argb{ 0xffd06d5f }, 34 },
        { Argb{ 0xff561c54 }, 27 }, { Argb{ 0xff713090 }, 88 } };
    expectRanked(score(colours, { .desired = 5, .fallbackColour = Argb{ 0xff58c19c }, .filter = false }),
        { 0xffdf241c, 0xff561c54 });
}

TEST(Score, GeneratedScenarioFive) {
    const Population colours{ { Argb{ 0xffbe66f8 }, 41 }, { Argb{ 0xff4bbda9 }, 88 }, { Argb{ 0xff80f6f9 }, 44 },
        { Argb{ 0xffab8017 }, 43 }, { Argb{ 0xffe89307 }, 65 } };
    expectRanked(score(colours, { .desired = 3, .fallbackColour = Argb{ 0xff916691 }, .filter = false }),
        { 0xffab8017, 0xff4bbda9, 0xffbe66f8 });
}

TEST(Score, GeneratedScenarioSix) {
    const Population colours{ { Argb{ 0xff18ea8f }, 93 }, { Argb{ 0xff327593 }, 18 }, { Argb{ 0xff066a18 }, 53 },
        { Argb{ 0xfffa8a23 }, 74 }, { Argb{ 0xff04ca1f }, 62 } };
    expectRanked(score(colours, { .desired = 2, .fallbackColour = Argb{ 0xff4c377a }, .filter = false }),
        { 0xff18ea8f, 0xfffa8a23 });
}

TEST(Score, GeneratedScenarioSeven) {
    const Population colours{ { Argb{ 0xff2e05ed }, 23 }, { Argb{ 0xff153e55 }, 90 }, { Argb{ 0xff9ab220 }, 23 },
        { Argb{ 0xff153379 }, 66 }, { Argb{ 0xff68bcc3 }, 81 } };
    expectRanked(score(colours, { .desired = 2, .fallbackColour = Argb{ 0xfff588dc }, .filter = true }),
        { 0xff2e05ed, 0xff9ab220 });
}

TEST(Score, GeneratedScenarioEight) {
    const Population colours{ { Argb{ 0xff816ec5 }, 24 }, { Argb{ 0xff6dcb94 }, 19 }, { Argb{ 0xff3cae91 }, 98 },
        { Argb{ 0xff5b542f }, 25 } };
    expectRanked(
        score(colours, { .desired = 1, .fallbackColour = Argb{ 0xff84b0fd }, .filter = false }), { 0xff3cae91 });
}

TEST(Score, GeneratedScenarioNine) {
    const Population colours{ { Argb{ 0xff206f86 }, 52 }, { Argb{ 0xff4a620d }, 96 }, { Argb{ 0xfff51401 }, 85 },
        { Argb{ 0xff2b8ebf }, 3 }, { Argb{ 0xff277766 }, 59 } };
    expectRanked(score(colours, { .desired = 3, .fallbackColour = Argb{ 0xff02b415 }, .filter = true }),
        { 0xfff51401, 0xff4a620d, 0xff2b8ebf });
}

TEST(Score, GeneratedScenarioTen) {
    const Population colours{ { Argb{ 0xff8b1d99 }, 54 }, { Argb{ 0xff27effe }, 43 }, { Argb{ 0xff6f558d }, 2 },
        { Argb{ 0xff77fdf2 }, 78 } };
    expectRanked(score(colours, { .desired = 4, .fallbackColour = Argb{ 0xff5e7a10 }, .filter = true }),
        { 0xff27effe, 0xff8b1d99, 0xff6f558d });
}

} // namespace

} // namespace m3colours::score
