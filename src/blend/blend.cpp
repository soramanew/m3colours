#include "m3colours/blend.hpp"

#include <algorithm>

#include "m3colours/hct.hpp"

namespace m3colours::blend {

namespace {

hct::internal::Jch jchFromArgb(Argb argb) {
    return hct::internal::jchFromXyz(hct::internal::xyzFromArgb(argb), hct::k_defaultViewingConditions);
}

} // namespace

Argb harmonise(Argb designColour, Argb sourceColour, double maxRotation) {
    const auto fromXyz = hct::internal::xyzFromArgb(designColour);
    const auto from = hct::internal::jchFromXyz(fromXyz, hct::k_defaultViewingConditions);
    const auto toHue = jchFromArgb(sourceColour).hue;
    const auto rotationDegrees = std::min(differenceDegrees(from.hue, toHue) * 0.5, maxRotation);
    const auto outputHue = sanitiseDegrees(from.hue + rotationDegrees * rotationDirection(from.hue, toHue));
    return hct::solveToArgb(outputHue, from.chroma, lstarFromY(fromXyz.b));
}

Argb hctHue(Argb from, Argb to, double amount) {
    const auto ucsHue = jchFromArgb(cam16Ucs(from, to, amount)).hue;
    const auto fromXyz = hct::internal::xyzFromArgb(from);
    const auto fromChroma = hct::internal::jchFromXyz(fromXyz, hct::k_defaultViewingConditions).chroma;
    return hct::solveToArgb(ucsHue, fromChroma, lstarFromY(fromXyz.b));
}

Argb cam16Ucs(Argb from, Argb to, double amount) {
    const auto fromCam = hct::Cam16::fromArgb(from);
    const auto toCam = hct::Cam16::fromArgb(to);
    const auto jstar = fromCam.jstar + (toCam.jstar - fromCam.jstar) * amount;
    const auto astar = fromCam.astar + (toCam.astar - fromCam.astar) * amount;
    const auto bstar = fromCam.bstar + (toCam.bstar - fromCam.bstar) * amount;
    return hct::Cam16::fromUcs(jstar, astar, bstar).toArgb();
}

} // namespace m3colours::blend
