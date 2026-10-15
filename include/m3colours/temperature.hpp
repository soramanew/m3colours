#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "common.hpp"
#include "hct.hpp"

namespace m3colours::temperature {

class TemperatureCache {
public:
    explicit TemperatureCache(const Hct& input);

    [[nodiscard]] const Hct& input() const { return m_input; }

    [[nodiscard]] Hct warmest() const;
    [[nodiscard]] Hct coldest() const;
    [[nodiscard]] Hct complement() const;
    [[nodiscard]] std::vector<Hct> analogous(int count = 5, int divisions = 12) const;
    [[nodiscard]] double relativeTemperature(const Hct& hct) const;

    [[nodiscard]] double inputRelativeTemperature() const { return relativeTemperature(m_inputTemp); }

    static double rawTemperature(Argb argb);

private:
    static constexpr std::size_t k_hueCount = 361;

    [[nodiscard]] double relativeTemperature(double rawTemp) const;

    [[nodiscard]] Hct hctAtHue(std::size_t hue) const { return Hct(m_argbByHue[hue]); }

    Hct m_input;
    double m_inputTemp;
    std::vector<Argb> m_argbByHue;
    std::array<double, k_hueCount> m_tempByHue;
    std::size_t m_coldestHue;
    std::size_t m_warmestHue;
    double m_coldestTemp;
    double m_warmestTemp;
};

} // namespace m3colours::temperature
