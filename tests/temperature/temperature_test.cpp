#include <gtest/gtest.h>

#include <vector>

#include "m3colours/temperature.hpp"

namespace m3colours::temperature {

namespace {

constexpr Argb k_red{ 0xffff0000 };
constexpr Argb k_green{ 0xff00ff00 };
constexpr Argb k_blue{ 0xff0000ff };
constexpr Argb k_white{ 0xffffffff };
constexpr Argb k_black{ 0xff000000 };

double inputRelativeTemperature(Argb argb) {
    return TemperatureCache(hct::Hct(argb)).inputRelativeTemperature();
}

Argb complement(Argb argb) {
    return TemperatureCache(hct::Hct(argb)).complement().toArgb();
}

void expectAnalogous(Argb argb, const std::vector<Argb>& expected) {
    const auto analogous = TemperatureCache(hct::Hct(argb)).analogous();
    ASSERT_EQ(analogous.size(), expected.size());
    for (std::size_t i = 0; i < expected.size(); i++)
        EXPECT_EQ(analogous[i].toArgb(), expected[i]) << "index " << i;
}

TEST(RawTemperature, Blue) {
    EXPECT_NEAR(TemperatureCache::rawTemperature(k_blue), -1.393, 0.001);
}

TEST(RawTemperature, Red) {
    EXPECT_NEAR(TemperatureCache::rawTemperature(k_red), 2.351, 0.001);
}

TEST(RawTemperature, Green) {
    EXPECT_NEAR(TemperatureCache::rawTemperature(k_green), -0.267, 0.001);
}

TEST(RawTemperature, White) {
    EXPECT_NEAR(TemperatureCache::rawTemperature(k_white), -0.5, 0.001);
}

TEST(RawTemperature, Black) {
    EXPECT_NEAR(TemperatureCache::rawTemperature(k_black), -0.5, 0.001);
}

TEST(RelativeTemperature, Blue) {
    EXPECT_NEAR(inputRelativeTemperature(k_blue), 0.0, 0.001);
}

TEST(RelativeTemperature, Red) {
    EXPECT_NEAR(inputRelativeTemperature(k_red), 1.0, 0.001);
}

TEST(RelativeTemperature, Green) {
    EXPECT_NEAR(inputRelativeTemperature(k_green), 0.467, 0.001);
}

TEST(RelativeTemperature, White) {
    EXPECT_NEAR(inputRelativeTemperature(k_white), 0.5, 0.001);
}

TEST(RelativeTemperature, Black) {
    EXPECT_NEAR(inputRelativeTemperature(k_black), 0.5, 0.001);
}

TEST(Complement, Blue) {
    EXPECT_EQ(complement(k_blue), Argb{ 0xff9d0002 });
}

TEST(Complement, Red) {
    EXPECT_EQ(complement(k_red), Argb{ 0xff007bfc });
}

TEST(Complement, Green) {
    EXPECT_EQ(complement(k_green), Argb{ 0xffffd2c9 });
}

TEST(Complement, White) {
    EXPECT_EQ(complement(k_white), k_white);
}

TEST(Complement, Black) {
    EXPECT_EQ(complement(k_black), k_black);
}

TEST(Analogous, Blue) {
    expectAnalogous(
        k_blue, { Argb{ 0xff00590c }, Argb{ 0xff00564e }, Argb{ 0xff0000ff }, Argb{ 0xff6700cc }, Argb{ 0xff81009f } });
}

TEST(Analogous, Red) {
    expectAnalogous(
        k_red, { Argb{ 0xfff60082 }, Argb{ 0xfffc004c }, Argb{ 0xffff0000 }, Argb{ 0xffd95500 }, Argb{ 0xffaf7200 } });
}

TEST(Analogous, Green) {
    expectAnalogous(k_green,
        { Argb{ 0xffcee900 }, Argb{ 0xff92f500 }, Argb{ 0xff00ff00 }, Argb{ 0xff00fd6f }, Argb{ 0xff00fab3 } });
}

TEST(Analogous, Black) {
    expectAnalogous(k_black, { k_black, k_black, k_black, k_black, k_black });
}

TEST(Analogous, White) {
    expectAnalogous(k_white, { k_white, k_white, k_white, k_white, k_white });
}

} // namespace

} // namespace m3colours::temperature
