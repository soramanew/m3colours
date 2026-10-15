#pragma once

#include <cstdint>
#include <map>
#include <utility>
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

struct QuantiserResult {
    std::map<Argb, uint32_t> colourToCount;
    std::vector<std::pair<Argb, Argb>> inputToCluster;
};

struct WeightedPixels {
    std::vector<Argb> pixels;
    std::vector<std::uint32_t> counts;
};

namespace internal {

WeightedPixels uniquePixels(const std::vector<Argb>& pixels, bool opaqueOnly);
std::vector<Argb> wu(const WeightedPixels& pixels, uint16_t maxColours);
QuantiserResult wsMeans(const WeightedPixels& pixels, const std::vector<Argb>& startingClusters, uint16_t maxColours);

} // namespace internal

std::vector<Argb> wu(const std::vector<Argb>& pixels, uint16_t maxColours);

QuantiserResult wsMeans(
    const std::vector<Argb>& inputPixels, const std::vector<Argb>& startingClusters, uint16_t maxColours);

QuantiserResult celebi(const std::vector<Argb>& pixels, uint16_t maxColours, bool filterTransparent = true);

} // namespace m3colours::quantise
