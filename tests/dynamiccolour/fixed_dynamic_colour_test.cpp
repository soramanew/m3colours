#include <gtest/gtest.h>

#include <vector>

#include "m3colours/dynamiccolour.hpp"

namespace m3colours::dynamiccolour {

namespace {

constexpr Argb k_red{ 0xffff0000 };

DynamicScheme makeScheme(Variant variant, bool isDark) {
    return DynamicScheme{ { .sourceColourHcts = { Hct{ k_red } }, .variant = variant, .isDark = isDark } };
}

struct ExpectedTone {
    const DynamicColour& colour;
    double tone;
};

void expectFixedTones(const DynamicScheme& scheme, const std::vector<ExpectedTone>& expected) {
    for (const auto& [colour, tone] : expected)
        EXPECT_NEAR(colour.getHct(scheme).tone(), tone, 0.5) << colour.name();
}

const std::vector<ExpectedTone> k_monochromeFixedTones{
    { .colour = material::primaryFixed(), .tone = 40.0 },
    { .colour = material::primaryFixedDim(), .tone = 30.0 },
    { .colour = material::onPrimaryFixed(), .tone = 100.0 },
    { .colour = material::onPrimaryFixedVariant(), .tone = 90.0 },
    { .colour = material::secondaryFixed(), .tone = 80.0 },
    { .colour = material::secondaryFixedDim(), .tone = 70.0 },
    { .colour = material::onSecondaryFixed(), .tone = 10.0 },
    { .colour = material::onSecondaryFixedVariant(), .tone = 25.0 },
    { .colour = material::tertiaryFixed(), .tone = 40.0 },
    { .colour = material::tertiaryFixedDim(), .tone = 30.0 },
    { .colour = material::onTertiaryFixed(), .tone = 100.0 },
    { .colour = material::onTertiaryFixedVariant(), .tone = 90.0 },
};

const std::vector<ExpectedTone> k_tonalSpotFixedTones{
    { .colour = material::primaryFixed(), .tone = 90.0 },
    { .colour = material::primaryFixedDim(), .tone = 80.0 },
    { .colour = material::onPrimaryFixed(), .tone = 10.0 },
    { .colour = material::onPrimaryFixedVariant(), .tone = 30.0 },
    { .colour = material::secondaryFixed(), .tone = 90.0 },
    { .colour = material::secondaryFixedDim(), .tone = 80.0 },
    { .colour = material::onSecondaryFixed(), .tone = 10.0 },
    { .colour = material::onSecondaryFixedVariant(), .tone = 30.0 },
    { .colour = material::tertiaryFixed(), .tone = 90.0 },
    { .colour = material::tertiaryFixedDim(), .tone = 80.0 },
    { .colour = material::onTertiaryFixed(), .tone = 10.0 },
    { .colour = material::onTertiaryFixedVariant(), .tone = 30.0 },
};

TEST(FixedColours, TonesInNonMonochromeScheme) {
    expectFixedTones(makeScheme(Variant::TonalSpot, true), k_tonalSpotFixedTones);
}

TEST(FixedColours, ArgbInNonMonochromeScheme) {
    const auto scheme = makeScheme(Variant::TonalSpot, true);

    EXPECT_EQ(scheme.primaryFixed(), Argb{ 0xffffdad4 });
    EXPECT_EQ(scheme.primaryFixedDim(), Argb{ 0xffffb4a8 });
    EXPECT_EQ(scheme.onPrimaryFixed(), Argb{ 0xff3a0905 });
    EXPECT_EQ(scheme.onPrimaryFixedVariant(), Argb{ 0xff73342a });
    EXPECT_EQ(scheme.secondaryFixed(), Argb{ 0xffffdad4 });
    EXPECT_EQ(scheme.secondaryFixedDim(), Argb{ 0xffe7bdb6 });
    EXPECT_EQ(scheme.onSecondaryFixed(), Argb{ 0xff2c1512 });
    EXPECT_EQ(scheme.onSecondaryFixedVariant(), Argb{ 0xff5d3f3b });
    EXPECT_EQ(scheme.tertiaryFixed(), Argb{ 0xfffbdfa6 });
    EXPECT_EQ(scheme.tertiaryFixedDim(), Argb{ 0xffdec48c });
    EXPECT_EQ(scheme.onTertiaryFixed(), Argb{ 0xff251a00 });
    EXPECT_EQ(scheme.onTertiaryFixedVariant(), Argb{ 0xff564419 });
}

TEST(FixedColours, LightMonochromeScheme) {
    expectFixedTones(makeScheme(Variant::Monochrome, false), k_monochromeFixedTones);
}

TEST(FixedColours, DarkMonochromeScheme) {
    expectFixedTones(makeScheme(Variant::Monochrome, true), k_monochromeFixedTones);
}

} // namespace

} // namespace m3colours::dynamiccolour
