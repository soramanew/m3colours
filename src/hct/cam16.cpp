#include <cmath>
#include <numbers>

#include "m3colours/hct.hpp"

namespace m3colours::hct {

namespace {

constexpr double k_degreesPerRadian = 180.0 / std::numbers::pi;
constexpr double k_radiansPerDegree = std::numbers::pi / 180.0;

double chromaticAdaptation(double component, double fl) {
    const auto af = std::pow(fl * std::abs(component) / 100.0, 0.42);
    return std::copysign(400.0 * af / (af + 27.13), component);
}

double inverseChromaticAdaptation(double adapted, double fl) {
    const auto adaptedAbs = std::abs(adapted);
    const auto base = std::max(0.0, 27.13 * adaptedAbs / (400.0 - adaptedAbs));
    return std::copysign((100.0 / fl) * std::pow(base, 1.0 / 0.42), adapted);
}

} // namespace

Vec3 internal::xyzFromArgb(Argb argb) {
    const auto redL = detail::linearise(argb.red());
    const auto greenL = detail::linearise(argb.green());
    const auto blueL = detail::linearise(argb.blue());

    return {
        .a = 0.41233895 * redL + 0.35762064 * greenL + 0.18051042 * blueL,
        .b = 0.2126 * redL + 0.7152 * greenL + 0.0722 * blueL,
        .c = 0.01932141 * redL + 0.11916382 * greenL + 0.95034478 * blueL,
    };
}

internal::Jch internal::jchFromXyz(Vec3 xyz, const ViewingConditions& vc) {
    const auto rC = 0.401288 * xyz.a + 0.650173 * xyz.b - 0.051461 * xyz.c;
    const auto gC = -0.250268 * xyz.a + 1.204414 * xyz.b + 0.045854 * xyz.c;
    const auto bC = -0.002079 * xyz.a + 0.048952 * xyz.b + 0.953127 * xyz.c;

    const auto rA = chromaticAdaptation(vc.rgbD[0] * rC, vc.fl);
    const auto gA = chromaticAdaptation(vc.rgbD[1] * gC, vc.fl);
    const auto bA = chromaticAdaptation(vc.rgbD[2] * bC, vc.fl);

    const auto a = (11.0 * rA + -12.0 * gA + bA) / 11.0;
    const auto b = (rA + gA - 2.0 * bA) / 9.0;
    const auto u = (20.0 * rA + 20.0 * gA + 21.0 * bA) / 20.0;
    const auto p2 = (40.0 * rA + 20.0 * gA + bA) / 20.0;

    const auto atanDegrees = std::atan2(b, a) * k_degreesPerRadian;
    const auto hue = atanDegrees < 0.0 ? atanDegrees + 360.0 : atanDegrees;
    const auto j = 100.0 * std::pow(p2 * vc.nbb / vc.aw, vc.c * vc.z);

    const auto huePrime = hue < 20.14 ? hue + 360.0 : hue;
    const auto eHue = 0.25 * (std::cos(huePrime * k_radiansPerDegree + 2.0) + 3.8);
    const auto p1 = (50000.0 / 13.0) * eHue * vc.nc * vc.ncb;
    const auto t = (p1 * std::sqrt(a * a + b * b)) / (u + 0.305);
    const auto alpha = std::pow(t, 0.9) * vc.alphaCoeff;

    return { .j = j, .chroma = alpha * std::sqrt(j / 100.0), .hue = hue };
}

Vec3 internal::xyzFromJch(Jch jch, const ViewingConditions& vc) {
    const auto [j, chroma, hue] = jch;
    const auto alpha = chroma <= 0.0 || j <= 0.0 ? 0.0 : chroma / std::sqrt(j / 100.0);
    const auto t = std::pow(alpha / vc.alphaCoeff, 1.0 / 0.9);
    const auto hRad = hue * k_radiansPerDegree;
    const auto eHue = 0.25 * (std::cos(hRad + 2.0) + 3.8);
    const auto ac = vc.aw * std::pow(j / 100.0, 1.0 / vc.c / vc.z);
    const auto p1 = eHue * (50000.0 / 13.0) * vc.nc * vc.ncb;
    const auto p2 = ac / vc.nbb;
    const auto hSin = std::sin(hRad);
    const auto hCos = std::cos(hRad);
    const auto gamma = (23.0 * (p2 + 0.305) * t) / (23.0 * p1 + 11.0 * t * hCos + 108.0 * t * hSin);
    const auto a = gamma * hCos;
    const auto b = gamma * hSin;

    const auto rA = (460.0 * p2 + 451.0 * a + 288.0 * b) / 1403.0;
    const auto gA = (460.0 * p2 - 891.0 * a - 261.0 * b) / 1403.0;
    const auto bA = (460.0 * p2 - 220.0 * a - 6300.0 * b) / 1403.0;

    const auto rF = inverseChromaticAdaptation(rA, vc.fl) / vc.rgbD[0];
    const auto gF = inverseChromaticAdaptation(gA, vc.fl) / vc.rgbD[1];
    const auto bF = inverseChromaticAdaptation(bA, vc.fl) / vc.rgbD[2];

    return {
        .a = 1.86206786 * rF - 1.01125463 * gF + 0.14918677 * bF,
        .b = 0.38752654 * rF + 0.62144744 * gF - 0.00897398 * bF,
        .c = -0.01584150 * rF - 0.03412294 * gF + 1.04996444 * bF,
    };
}

Cam16 Cam16::fromArgb(Argb argb, const ViewingConditions& vc) {
    return fromXyz(internal::xyzFromArgb(argb), vc);
}

Cam16 Cam16::fromXyz(Vec3 xyz, const ViewingConditions& vc) {
    const auto [j, chroma, hue] = internal::jchFromXyz(xyz, vc);
    return fromJch(j, chroma, hue, vc);
}

Cam16 Cam16::fromJch(double j, double c, double h, const ViewingConditions& vc) {
    const auto jRoot = std::sqrt(j / 100.0);
    const auto q = (4.0 / vc.c) * jRoot * (vc.aw + 4.0) * vc.flRoot;
    const auto m = c * vc.flRoot;
    const auto alpha = j <= 0.0 ? 0.0 : c / jRoot;
    const auto s = 50.0 * std::sqrt((alpha * vc.c) / (vc.aw + 4.0));
    const auto hueRadians = h * k_radiansPerDegree;
    const auto jstar = ((1.0 + 100.0 * 0.007) * j) / (1.0 + 0.007 * j);
    const auto mstar = (1.0 / 0.0228) * std::log(1.0 + 0.0228 * m);

    return {
        .hue = h,
        .chroma = c,
        .j = j,
        .q = q,
        .m = m,
        .s = s,
        .jstar = jstar,
        .astar = mstar * std::cos(hueRadians),
        .bstar = mstar * std::sin(hueRadians),
    };
}

Cam16 Cam16::fromUcs(double jstar, double astar, double bstar, const ViewingConditions& vc) {
    const auto mstar = std::sqrt(astar * astar + bstar * bstar);
    const auto m = (std::exp(mstar * 0.0228) - 1.0) / 0.0228;
    const auto c = m / vc.flRoot;
    auto h = std::atan2(bstar, astar) * k_degreesPerRadian;
    if (h < 0.0)
        h += 360.0;
    const auto j = jstar / (1.0 - (jstar - 100.0) * 0.007);
    return fromJch(j, c, h, vc);
}

double Cam16::distance(const Cam16& other) const {
    const auto dJ = jstar - other.jstar;
    const auto dA = astar - other.astar;
    const auto dB = bstar - other.bstar;
    const auto dEPrime = std::sqrt(dJ * dJ + dA * dA + dB * dB);
    return 1.41 * std::pow(dEPrime, 0.63);
}

Vec3 Cam16::toXyz(const ViewingConditions& vc) const {
    return internal::xyzFromJch({ .j = j, .chroma = chroma, .hue = hue }, vc);
}

Argb Cam16::toArgb(const ViewingConditions& vc) const {
    const auto [x, y, z] = toXyz(vc);
    return Argb(Vec3{
        .a = 3.2413774792388685 * x - 1.5376652402851851 * y - 0.49885366846268053 * z,
        .b = -0.9691452513005321 * x + 1.8758853451067872 * y + 0.04156585616912061 * z,
        .c = 0.05562093689691305 * x - 0.20395524564742123 * y + 1.0571799111220335 * z,
    });
}

} // namespace m3colours::hct
