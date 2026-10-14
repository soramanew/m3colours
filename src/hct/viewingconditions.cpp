#include <cmath>

#include "m3colours/hct.hpp"

namespace m3colours::hct {

namespace {

constexpr double lerp(double start, double stop, double amount) {
    return (1.0 - amount) * start + amount * stop;
}

} // namespace

ViewingConditions ViewingConditions::make(std::array<double, 3> whitePoint, double adaptingLuminance,
    double backgroundLstar, double surround, bool discountingIlluminant) {
    const auto rW = whitePoint[0] * 0.401288 + whitePoint[1] * 0.650173 + whitePoint[2] * -0.051461;
    const auto gW = whitePoint[0] * -0.250268 + whitePoint[1] * 1.204414 + whitePoint[2] * 0.045854;
    const auto bW = whitePoint[0] * -0.002079 + whitePoint[1] * 0.048952 + whitePoint[2] * 0.953127;

    const auto f = 0.8 + surround / 10.0;
    const auto c = f >= 0.9 ? lerp(0.59, 0.69, (f - 0.9) * 10.0) : lerp(0.525, 0.59, (f - 0.8) * 10.0);
    const auto d = discountingIlluminant
                       ? 1.0
                       : std::clamp(f * (1.0 - (1.0 / 3.6) * std::exp((-adaptingLuminance - 42.0) / 92.0)), 0.0, 1.0);

    const std::array<double, 3> rgbD{
        d * (100.0 / rW) + 1.0 - d,
        d * (100.0 / gW) + 1.0 - d,
        d * (100.0 / bW) + 1.0 - d,
    };

    const auto k = 1.0 / (5.0 * adaptingLuminance + 1.0);
    const auto k4 = k * k * k * k;
    const auto k4F = 1.0 - k4;
    const auto fl = k4 * adaptingLuminance + 0.1 * k4F * k4F * std::cbrt(5.0 * adaptingLuminance);

    const auto n = yFromLstar(backgroundLstar) / whitePoint[1];
    const auto nbb = 0.725 / std::pow(n, 0.2);

    const auto rAF = std::pow((fl * rgbD[0] * rW) / 100.0, 0.42);
    const auto gAF = std::pow((fl * rgbD[1] * gW) / 100.0, 0.42);
    const auto bAF = std::pow((fl * rgbD[2] * bW) / 100.0, 0.42);
    const auto rA = (400.0 * rAF) / (rAF + 27.13);
    const auto gA = (400.0 * gAF) / (gAF + 27.13);
    const auto bA = (400.0 * bAF) / (bAF + 27.13);
    const auto aw = (2.0 * rA + gA + 0.05 * bA) * nbb;

    return {
        .n = n,
        .aw = aw,
        .nbb = nbb,
        .ncb = nbb,
        .c = c,
        .nc = f,
        .rgbD = rgbD,
        .fl = fl,
        .flRoot = std::pow(fl, 0.25),
        .z = 1.48 + std::sqrt(n),
        .alphaCoeff = std::pow(1.64 - std::pow(0.29, n), 0.73),
    };
}

} // namespace m3colours::hct
