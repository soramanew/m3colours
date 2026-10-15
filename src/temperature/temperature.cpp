#include "m3colours/temperature.hpp"

#include <cmath>
#include <numbers>

#include "m3colours/quantise.hpp"

namespace m3colours::temperature {

namespace {

bool isBetween(double angle, double a, double b) {
    if (a < b)
        return a <= angle && angle <= b;
    return a <= angle || angle <= b;
}

std::size_t hueIndex(double hue) {
    return static_cast<std::size_t>(std::lround(hue));
}

} // namespace

TemperatureCache::TemperatureCache(const Hct& input)
    : m_input(input)
    , m_inputTemp(rawTemperature(input.toArgb()))
    , m_tempByHue()
    , m_coldestHue(0)
    , m_warmestHue(0)
    , m_coldestTemp(m_inputTemp)
    , m_warmestTemp(m_inputTemp) {
    const auto chroma = input.chroma();
    const auto tone = input.tone();
    m_argbByHue.reserve(k_hueCount);

    for (std::size_t hue = 0; hue < k_hueCount; hue++) {
        const auto argb = hct::solveToArgb(static_cast<double>(hue), chroma, tone);
        const auto temp = rawTemperature(argb);
        m_argbByHue.push_back(argb);
        m_tempByHue[hue] = temp;

        if (hue == 0 || temp < m_coldestTemp) {
            m_coldestTemp = temp;
            m_coldestHue = hue;
        }
        if (hue == 0 || temp >= m_warmestTemp) {
            m_warmestTemp = temp;
            m_warmestHue = hue;
        }
    }

    if (m_inputTemp >= m_warmestTemp) {
        m_warmestTemp = m_inputTemp;
        m_warmestHue = k_hueCount;
    }
}

Hct TemperatureCache::warmest() const {
    return m_warmestHue == k_hueCount ? m_input : hctAtHue(m_warmestHue);
}

Hct TemperatureCache::coldest() const {
    return hctAtHue(m_coldestHue);
}

std::vector<Hct> TemperatureCache::analogous(int count, int divisions) const {
    const auto startHue = hueIndex(m_input.hue());
    const auto startTemp = relativeTemperature(m_tempByHue[startHue]);

    auto lastTemp = startTemp;
    auto absoluteTotalTempDelta = 0.0;
    for (std::size_t i = 0; i < 360; i++) {
        const auto temp = relativeTemperature(m_tempByHue[(startHue + i) % 360]);
        absoluteTotalTempDelta += std::abs(temp - lastTemp);
        lastTemp = temp;
    }

    const auto divisionCount = static_cast<std::size_t>(divisions);
    std::vector<std::size_t> allHues;
    allHues.reserve(divisionCount);
    allHues.push_back(startHue);

    const auto tempStep = absoluteTotalTempDelta / divisions;
    auto totalTempDelta = 0.0;
    lastTemp = startTemp;
    std::size_t hueAddend = 1;
    while (allHues.size() < divisionCount) {
        const auto hue = (startHue + hueAddend) % 360;
        const auto temp = relativeTemperature(m_tempByHue[hue]);
        totalTempDelta += std::abs(temp - lastTemp);

        auto indexSatisfied = totalTempDelta >= static_cast<double>(allHues.size()) * tempStep;
        std::size_t indexAddend = 1;
        while (indexSatisfied && allHues.size() < divisionCount) {
            allHues.push_back(hue);
            indexSatisfied = totalTempDelta >= static_cast<double>(allHues.size() + indexAddend) * tempStep;
            indexAddend++;
        }

        lastTemp = temp;
        hueAddend++;
        if (hueAddend > 360) {
            while (allHues.size() < divisionCount)
                allHues.push_back(hue);
            break;
        }
    }

    const auto size = static_cast<int>(allHues.size());
    const auto wrap = [size](int index) {
        return static_cast<std::size_t>(((index % size) + size) % size);
    };

    std::vector<Hct> answers;
    answers.reserve(static_cast<std::size_t>(count));

    const auto increaseHueCount = (count - 1) / 2;
    for (int i = increaseHueCount; i >= 1; i--)
        answers.push_back(hctAtHue(allHues[wrap(-i)]));

    answers.push_back(m_input);

    const auto decreaseHueCount = count - increaseHueCount - 1;
    for (int i = 1; i <= decreaseHueCount; i++)
        answers.push_back(hctAtHue(allHues[wrap(i)]));

    return answers;
}

Hct TemperatureCache::complement() const {
    const auto range = m_warmestTemp - m_coldestTemp;
    const auto inputHueIndex = hueIndex(m_input.hue());
    if (range <= 0.0)
        return hctAtHue(inputHueIndex);

    const auto coldestHue = coldest().hue();
    const auto warmestHue = warmest().hue();
    const auto startHueIsColdestToWarmest = isBetween(m_input.hue(), coldestHue, warmestHue);
    const auto startHue = startHueIsColdestToWarmest ? warmestHue : coldestHue;
    const auto endHue = startHueIsColdestToWarmest ? coldestHue : warmestHue;

    const auto complementRelativeTemp = 1.0 - inputRelativeTemperature();
    auto smallestError = 1000.0;
    auto answer = inputHueIndex;

    for (int hueAddend = 0; hueAddend <= 360; hueAddend++) {
        const auto hue = sanitiseDegrees(startHue + hueAddend);
        if (!isBetween(hue, startHue, endHue))
            continue;

        const auto index = hueIndex(hue);
        const auto relativeTemp = (m_tempByHue[index] - m_coldestTemp) / range;
        const auto error = std::abs(complementRelativeTemp - relativeTemp);
        if (error < smallestError) {
            smallestError = error;
            answer = index;
        }
    }

    return hctAtHue(answer);
}

double TemperatureCache::relativeTemperature(const Hct& hct) const {
    return relativeTemperature(rawTemperature(hct.toArgb()));
}

double TemperatureCache::relativeTemperature(double rawTemp) const {
    const auto range = m_warmestTemp - m_coldestTemp;
    if (range <= 0.0)
        return 0.5;
    return (rawTemp - m_coldestTemp) / range;
}

double TemperatureCache::rawTemperature(Argb argb) {
    const quantise::Lab lab(argb);
    const auto hue = std::atan2(lab.b, lab.a) * 180.0 / std::numbers::pi;
    const auto chroma = std::hypot(lab.a, lab.b);
    return -0.5 + 0.02 * std::pow(chroma, 1.07) * std::cos((hue - 50.0) * std::numbers::pi / 180.0);
}

} // namespace m3colours::temperature
