#include "m3colours/contrast.hpp"

#include <algorithm>
#include <cmath>

#include "m3colours/common.hpp"

namespace m3colours::contrast {

double ratioOfYs(double y1, double y2) {
    const auto lighter = std::max(y1, y2);
    const auto darker = std::min(y1, y2);
    return (lighter + 5.0) / (darker + 5.0);
}

double ratioOfTones(double toneA, double toneB) {
    toneA = std::clamp(toneA, 0.0, 100.0);
    toneB = std::clamp(toneB, 0.0, 100.0);
    return ratioOfYs(yFromLstar(toneA), yFromLstar(toneB));
}

double lighter(double tone, double ratio) {
    if (tone < 0.0 || tone > 100.0)
        return -1.0;

    const auto darkY = yFromLstar(tone);
    const auto lightY = ratio * (darkY + 5.0) - 5.0;
    const auto realContrast = ratioOfYs(lightY, darkY);
    if (realContrast < ratio && std::abs(realContrast - ratio) > 0.04)
        return -1.0;

    const auto result = lstarFromY(lightY) + 0.4;
    if (result < 0.0 || result > 100.0)
        return -1.0;
    return result;
}

double darker(double tone, double ratio) {
    if (tone < 0.0 || tone > 100.0)
        return -1.0;

    const auto lightY = yFromLstar(tone);
    const auto darkY = (lightY + 5.0) / ratio - 5.0;
    const auto realContrast = ratioOfYs(lightY, darkY);
    if (realContrast < ratio && std::abs(realContrast - ratio) > 0.04)
        return -1.0;

    const auto result = lstarFromY(darkY) - 0.4;
    if (result < 0.0 || result > 100.0)
        return -1.0;
    return result;
}

double lighterUnsafe(double tone, double ratio) {
    const auto safe = lighter(tone, ratio);
    return safe < 0.0 ? 100.0 : safe;
}

double darkerUnsafe(double tone, double ratio) {
    const auto safe = darker(tone, ratio);
    return safe < 0.0 ? 0.0 : safe;
}

} // namespace m3colours::contrast
