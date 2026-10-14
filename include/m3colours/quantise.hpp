#pragma once

#include <vector>

#include "common.hpp"

namespace m3colours::quantise {

struct Lab {
    double l;
    double a;
    double b;

    Lab(double l, double a, double b);

    [[nodiscard]] double deltaE(const Lab& lab) const {
        const auto dL = l - lab.l;
        const auto dA = a - lab.a;
        const auto dB = b - lab.b;
        return (dL * dL) + (dA * dA) + (dB * dB);
    }

    explicit Lab(Argb argb);
    [[nodiscard]] Argb toArgb() const;
};

std::vector<Argb> wu(const std::vector<Argb>& pixels, uint16_t maxColours);

} // namespace m3colours::quantise
