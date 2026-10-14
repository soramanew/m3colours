#pragma once

#include <array>
#include <numbers>

#include "common.hpp"

namespace m3colours::hct {

struct ViewingConditions {
    double n;
    double aw;
    double nbb;
    double ncb;
    double c;
    double nc;
    std::array<double, 3> rgbD;
    double fl;
    double flRoot;
    double z;

    static ViewingConditions make(std::array<double, 3> whitePoint = k_whitePointD65,
        double adaptingLuminance = (200.0 / std::numbers::pi) * yFromLstar(50.0) / 100.0, double backgroundLstar = 50.0,
        double surround = 2.0, bool discountingIlluminant = false);
};

inline const ViewingConditions k_defaultViewingConditions = ViewingConditions::make();

struct Cam16 {
    double hue;
    double chroma;
    double j;
    double q;
    double m;
    double s;
    double jstar;
    double astar;
    double bstar;

    static Cam16 fromArgb(Argb argb, const ViewingConditions& vc = k_defaultViewingConditions);
    static Cam16 fromXyz(Vec3 xyz, const ViewingConditions& vc = k_defaultViewingConditions);
    static Cam16 fromJch(double j, double c, double h, const ViewingConditions& vc = k_defaultViewingConditions);
    static Cam16 fromUcs(
        double jstar, double astar, double bstar, const ViewingConditions& vc = k_defaultViewingConditions);

    [[nodiscard]] double distance(const Cam16& other) const;
    [[nodiscard]] Vec3 toXyz(const ViewingConditions& vc = k_defaultViewingConditions) const;
    [[nodiscard]] Argb toArgb(const ViewingConditions& vc = k_defaultViewingConditions) const;
};

Argb solveToArgb(double hueDegrees, double chroma, double lstar);

class Hct {
public:
    Hct(double hue, double chroma, double tone);
    explicit Hct(Argb argb);

    // clang-format off
    [[nodiscard]] double hue() const { return m_hue; }
    [[nodiscard]] double chroma() const { return m_chroma; }
    [[nodiscard]] double tone() const { return m_tone; }
    [[nodiscard]] Argb toArgb() const { return m_argb; }
    // clang-format on

    void setHue(double hue);
    void setChroma(double chroma);
    void setTone(double tone);

    [[nodiscard]] Hct inViewingConditions(const ViewingConditions& vc) const;

    // clang-format off
    [[nodiscard]] static constexpr bool isBlue(double hue) { return hue >= 250.0 && hue < 270.0; }
    [[nodiscard]] static constexpr bool isYellow(double hue) { return hue >= 105.0 && hue < 125.0; }
    [[nodiscard]] static constexpr bool isCyan(double hue) { return hue >= 170.0 && hue < 207.0; }
    // clang-format on

private:
    void setArgb(Argb argb);

    Argb m_argb;
    double m_hue;
    double m_chroma;
    double m_tone;
};

} // namespace m3colours::hct
