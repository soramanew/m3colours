#include "m3colours/hct.hpp"

namespace m3colours::hct {

Hct::Hct(double hue, double chroma, double tone)
    : Hct(solveToArgb(hue, chroma, tone)) {}

Hct::Hct(Argb argb)
    : m_argb(argb) {
    setArgb(argb);
}

void Hct::setHue(double hue) {
    setArgb(solveToArgb(hue, m_chroma, m_tone));
}

void Hct::setChroma(double chroma) {
    setArgb(solveToArgb(m_hue, chroma, m_tone));
}

void Hct::setTone(double tone) {
    setArgb(solveToArgb(m_hue, m_chroma, tone));
}

Hct Hct::inViewingConditions(const ViewingConditions& vc) const {
    const auto jch = internal::jchFromXyz(internal::xyzFromArgb(m_argb), k_defaultViewingConditions);
    const auto viewedInVc = internal::xyzFromJch(jch, vc);
    const auto recastInVc = internal::jchFromXyz(viewedInVc, k_defaultViewingConditions);
    return { recastInVc.hue, recastInVc.chroma, lstarFromY(viewedInVc.b) };
}

void Hct::setArgb(Argb argb) {
    const auto xyz = internal::xyzFromArgb(argb);
    const auto [j, chroma, hue] = internal::jchFromXyz(xyz, k_defaultViewingConditions);
    m_argb = argb;
    m_hue = hue;
    m_chroma = chroma;
    m_tone = lstarFromY(xyz.b);
}

} // namespace m3colours::hct
