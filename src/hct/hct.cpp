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
    const auto viewedInVc = Cam16::fromArgb(m_argb).toXyz(vc);
    const auto recastInVc = Cam16::fromXyz(viewedInVc);
    return { recastInVc.hue, recastInVc.chroma, lstarFromY(viewedInVc.b) };
}

void Hct::setArgb(Argb argb) {
    const auto cam = Cam16::fromArgb(argb);
    m_argb = argb;
    m_hue = cam.hue;
    m_chroma = cam.chroma;
    m_tone = lstarFromArgb(argb);
}

} // namespace m3colours::hct
