#include <gtest/gtest.h>

#include "m3colours/dynamiccolour.hpp"

namespace m3colours::dynamiccolour {

namespace {

const Hct k_source{ 43.0, 16.0, 16.0 };

TEST(DynamicScheme, RotatedHueZeroLengthInput) {
    EXPECT_NEAR(DynamicScheme::getRotatedHue(k_source, {}, {}), 43.0, 0.2);
}

TEST(DynamicScheme, RotatedHueOneLengthInputNoRotation) {
    constexpr double k_breakpoints[]{ 0 };
    constexpr double k_rotations[]{ 0 };
    EXPECT_NEAR(DynamicScheme::getRotatedHue(k_source, k_breakpoints, k_rotations), 43.0, 0.2);
}

TEST(DynamicScheme, RotatedHueInputLengthMismatch) {
    constexpr double k_breakpoints[]{ 0 };
    constexpr double k_rotations[]{ 0, 1 };
    EXPECT_NEAR(DynamicScheme::getRotatedHue(k_source, k_breakpoints, k_rotations), 43.0, 0.2);
}

TEST(DynamicScheme, RotatedHueOnBoundary) {
    constexpr double k_breakpoints[]{ 0, 42, 360 };
    constexpr double k_rotations[]{ 0, 15, 0 };
    EXPECT_NEAR(DynamicScheme::getRotatedHue(k_source, k_breakpoints, k_rotations), 43.0 + 15.0, 0.2);
}

TEST(DynamicScheme, RotatedHueWrapsPast360) {
    constexpr double k_breakpoints[]{ 0, 42, 360 };
    constexpr double k_rotations[]{ 0, 480, 0 };
    EXPECT_NEAR(DynamicScheme::getRotatedHue(k_source, k_breakpoints, k_rotations), 163.0, 0.2);
}

TEST(DynamicScheme, RequiresSourceColour) {
    EXPECT_THROW(DynamicScheme{ { .sourceColourHcts = {} } }, std::invalid_argument);
}

TEST(DynamicScheme, DimColoursUndefinedBefore2025) {
    const DynamicScheme scheme{ { .sourceColourHcts = { k_source }, .variant = Variant::TonalSpot } };
    EXPECT_FALSE(scheme.primaryDim().has_value());

    const DynamicScheme scheme2025{
        { .sourceColourHcts = { k_source }, .variant = Variant::TonalSpot, .specVersion = SpecVersion::Spec2025 }
    };
    EXPECT_TRUE(scheme2025.primaryDim().has_value());
}

TEST(DynamicScheme, FallsBackToSpec2021ForUnsupportedVariants) {
    const DynamicScheme scheme{
        { .sourceColourHcts = { k_source }, .variant = Variant::Monochrome, .specVersion = SpecVersion::Spec2025 }
    };
    EXPECT_EQ(scheme.specVersion(), SpecVersion::Spec2021);
}

} // namespace

} // namespace m3colours::dynamiccolour
