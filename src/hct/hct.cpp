#include "m3colours/hct.hpp"

namespace m3colours {

Hct::Hct(double hue, double chroma, double tone)
    : Hct(hct::solveToArgb(hue, chroma, tone)) {}

Hct::Hct(Argb argb)
    : m_argb(argb) {
    setArgb(argb);
}

void Hct::setHue(double hue) {
    setArgb(hct::solveToArgb(hue, m_chroma, m_tone));
}

void Hct::setChroma(double chroma) {
    setArgb(hct::solveToArgb(m_hue, chroma, m_tone));
}

void Hct::setTone(double tone) {
    setArgb(hct::solveToArgb(m_hue, m_chroma, tone));
}

Hct Hct::inViewingConditions(const hct::ViewingConditions& vc) const {
    const auto jch = hct::internal::jchFromXyz(hct::internal::xyzFromArgb(m_argb), hct::k_defaultViewingConditions);
    const auto viewedInVc = hct::internal::xyzFromJch(jch, vc);
    const auto recastInVc = hct::internal::jchFromXyz(viewedInVc, hct::k_defaultViewingConditions);
    return { recastInVc.hue, recastInVc.chroma, lstarFromY(viewedInVc.b) };
}

void Hct::setArgb(Argb argb) {
    const auto xyz = hct::internal::xyzFromArgb(argb);
    const auto [j, chroma, hue] = hct::internal::jchFromXyz(xyz, hct::k_defaultViewingConditions);
    m_argb = argb;
    m_hue = hue;
    m_chroma = chroma;
    m_tone = lstarFromY(xyz.b);
}

} // namespace m3colours
