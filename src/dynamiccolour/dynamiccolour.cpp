#include "m3colours/dynamiccolour.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "m3colours/contrast.hpp"

namespace m3colours::dynamiccolour {

namespace {

double lerp(double start, double stop, double amount) {
    return start + (stop - start) * amount;
}

double clampDouble(double min, double max, double input) {
    if (input < min)
        return min;
    if (input > max)
        return max;
    return input;
}

bool isExactly(double value, double target) {
    return !(value < target) && !(value > target);
}

void validate(const std::string& name, const ColourSpec& spec) {
    if (!spec.background && spec.secondBackground)
        throw std::invalid_argument("Colour " + name + " has secondBackground defined, but background is not defined.");
    if (!spec.background && spec.contrastCurve)
        throw std::invalid_argument("Colour " + name + " has contrastCurve defined, but background is not defined.");
    if (spec.background && !spec.contrastCurve)
        throw std::invalid_argument("Colour " + name + " has background defined, but contrastCurve is not defined.");
}

double awkwardZoneAdjusted(double tone) {
    if (tone >= 57.0)
        return clampDouble(65.0, 100.0, tone);
    return clampDouble(0.0, 49.0, tone);
}

double dualBackgroundTone(const DynamicScheme& scheme, const DynamicColour& bg1, const DynamicColour& bg2,
    double answer, double desiredRatio) {
    const auto bgTone1 = bg1.getTone(scheme);
    const auto bgTone2 = bg2.getTone(scheme);
    const auto upper = std::max(bgTone1, bgTone2);
    const auto lower = std::min(bgTone1, bgTone2);

    if (contrast::ratioOfTones(upper, answer) >= desiredRatio && contrast::ratioOfTones(lower, answer) >= desiredRatio)
        return answer;

    const auto lightOption = contrast::lighter(upper, desiredRatio);
    const auto darkOption = contrast::darker(lower, desiredRatio);
    const auto hasLight = lightOption >= 0.0;
    const auto hasDark = darkOption >= 0.0;

    const auto prefersLight =
        DynamicColour::tonePrefersLightForeground(bgTone1) || DynamicColour::tonePrefersLightForeground(bgTone2);
    if (prefersLight)
        return hasLight ? lightOption : 100.0;
    if (hasLight != hasDark)
        return hasLight ? lightOption : darkOption;
    return hasDark ? darkOption : 0.0;
}

struct TonePair {
    double nearer;
    double farther;
};

TonePair contrastAdjusted2021(const DynamicScheme& scheme, const DynamicColour& colour, const DynamicColour& nearer,
    const DynamicColour& farther, TonePair tones) {
    const auto* bg = colour.background(scheme);
    const auto nCurve = nearer.contrastCurve(scheme);
    const auto fCurve = farther.contrastCurve(scheme);
    if (!bg || !nCurve || !fCurve)
        return tones;

    const auto bgTone = bg->getTone(scheme);
    const auto nContrast = nCurve->get(scheme.contrastLevel());
    const auto fContrast = fCurve->get(scheme.contrastLevel());
    const auto decreasingContrast = scheme.contrastLevel() < 0.0;
    if (decreasingContrast || contrast::ratioOfTones(bgTone, tones.nearer) < nContrast)
        tones.nearer = DynamicColour::foregroundTone(bgTone, nContrast);
    if (decreasingContrast || contrast::ratioOfTones(bgTone, tones.farther) < fContrast)
        tones.farther = DynamicColour::foregroundTone(bgTone, fContrast);
    return tones;
}

TonePair movedOutOfAwkwardZone(TonePair tones, double delta, double expansionDir, bool stayTogether) {
    const auto nearerInZone = 50.0 <= tones.nearer && tones.nearer < 60.0;
    const auto fartherInZone = 50.0 <= tones.farther && tones.farther < 60.0;
    if (nearerInZone || (fartherInZone && stayTogether)) {
        tones.nearer = expansionDir > 0.0 ? 60.0 : 49.0;
        const auto limit = tones.nearer + delta * expansionDir;
        tones.farther = expansionDir > 0.0 ? std::max(tones.farther, limit) : std::min(tones.farther, limit);
    } else if (fartherInZone) {
        tones.farther = expansionDir > 0.0 ? 60.0 : 49.0;
    }
    return tones;
}

double pairToneFor2021(const DynamicScheme& scheme, const DynamicColour& colour, const ToneDeltaPair& pair) {
    const auto aIsNearer = pair.polarity == TonePolarity::Nearer ||
                           (pair.polarity == TonePolarity::Lighter && !scheme.isDark()) ||
                           (pair.polarity == TonePolarity::Darker && scheme.isDark());
    const auto& nearer = aIsNearer ? pair.roleA : pair.roleB;
    const auto& farther = aIsNearer ? pair.roleB : pair.roleA;
    const auto expansionDir = scheme.isDark() ? 1.0 : -1.0;
    const auto delta = pair.delta;

    auto tones = contrastAdjusted2021(
        scheme, colour, nearer, farther, { .nearer = nearer.tone(scheme), .farther = farther.tone(scheme) });

    if ((tones.farther - tones.nearer) * expansionDir < delta) {
        tones.farther = clampDouble(0.0, 100.0, tones.nearer + delta * expansionDir);
        if ((tones.farther - tones.nearer) * expansionDir < delta)
            tones.nearer = clampDouble(0.0, 100.0, tones.farther - delta * expansionDir);
    }

    tones = movedOutOfAwkwardZone(tones, delta, expansionDir, pair.stayTogether);
    return colour.name() == nearer.name() ? tones.nearer : tones.farther;
}

double toneFor2021(const DynamicScheme& scheme, const DynamicColour& colour) {
    if (const auto pair = colour.toneDeltaPair(scheme))
        return pairToneFor2021(scheme, colour, *pair);

    const auto decreasingContrast = scheme.contrastLevel() < 0.0;
    auto answer = colour.tone(scheme);
    const auto* bg = colour.background(scheme);
    const auto curve = colour.contrastCurve(scheme);
    if (!bg || !curve)
        return answer;

    const auto bgTone = bg->getTone(scheme);
    const auto desiredRatio = curve->get(scheme.contrastLevel());

    if (contrast::ratioOfTones(bgTone, answer) < desiredRatio)
        answer = DynamicColour::foregroundTone(bgTone, desiredRatio);
    if (decreasingContrast)
        answer = DynamicColour::foregroundTone(bgTone, desiredRatio);

    if (colour.isBackground() && 50.0 <= answer && answer < 60.0)
        answer = contrast::ratioOfTones(49.0, bgTone) >= desiredRatio ? 49.0 : 60.0;

    const auto* bg2 = colour.secondBackground(scheme);
    if (!bg2)
        return answer;
    return dualBackgroundTone(scheme, *bg, *bg2, answer, desiredRatio);
}

double pairToneFor2025(const DynamicScheme& scheme, const DynamicColour& colour, const ToneDeltaPair& pair) {
    const auto absoluteDelta = pair.polarity == TonePolarity::Darker ||
                                       (pair.polarity == TonePolarity::RelativeLighter && scheme.isDark()) ||
                                       (pair.polarity == TonePolarity::RelativeDarker && !scheme.isDark())
                                   ? -pair.delta
                                   : pair.delta;

    const auto amRoleA = colour.name() == pair.roleA.name();
    const auto& selfRole = amRoleA ? pair.roleA : pair.roleB;
    const auto& refRole = amRoleA ? pair.roleB : pair.roleA;
    auto selfTone = selfRole.tone(scheme);
    const auto refTone = refRole.getTone(scheme);
    const auto relativeDelta = absoluteDelta * (amRoleA ? 1.0 : -1.0);

    switch (pair.constraint) {
    case DeltaConstraint::Exact:
        selfTone = clampDouble(0.0, 100.0, refTone + relativeDelta);
        break;
    case DeltaConstraint::Nearer:
        selfTone = relativeDelta > 0.0
                       ? clampDouble(0.0, 100.0, clampDouble(refTone, refTone + relativeDelta, selfTone))
                       : clampDouble(0.0, 100.0, clampDouble(refTone + relativeDelta, refTone, selfTone));
        break;
    case DeltaConstraint::Farther:
        selfTone = relativeDelta > 0.0 // format hack
                       ? clampDouble(refTone + relativeDelta, 100.0, selfTone)
                       : clampDouble(0.0, refTone + relativeDelta, selfTone);
        break;
    }

    if (const auto* bg = colour.background(scheme)) {
        if (const auto curve = colour.contrastCurve(scheme)) {
            const auto bgTone = bg->getTone(scheme);
            const auto selfContrast = curve->get(scheme.contrastLevel());
            if (contrast::ratioOfTones(bgTone, selfTone) < selfContrast || scheme.contrastLevel() < 0.0)
                selfTone = DynamicColour::foregroundTone(bgTone, selfContrast);
        }
    }

    if (colour.isBackground() && !colour.name().ends_with("_fixed_dim"))
        selfTone = awkwardZoneAdjusted(selfTone);

    return selfTone;
}

double toneFor2025(const DynamicScheme& scheme, const DynamicColour& colour) {
    if (const auto pair = colour.toneDeltaPair(scheme))
        return pairToneFor2025(scheme, colour, *pair);

    auto answer = colour.tone(scheme);
    const auto* bg = colour.background(scheme);
    const auto curve = colour.contrastCurve(scheme);
    if (!bg || !curve)
        return answer;

    const auto bgTone = bg->getTone(scheme);
    const auto desiredRatio = curve->get(scheme.contrastLevel());

    if (contrast::ratioOfTones(bgTone, answer) < desiredRatio || scheme.contrastLevel() < 0.0)
        answer = DynamicColour::foregroundTone(bgTone, desiredRatio);

    if (colour.isBackground() && !colour.name().ends_with("_fixed_dim"))
        answer = awkwardZoneAdjusted(answer);

    const auto* bg2 = colour.secondBackground(scheme);
    if (!bg2)
        return answer;
    return dualBackgroundTone(scheme, *bg, *bg2, answer, desiredRatio);
}

Hct hctFor2025(const DynamicScheme& scheme, const DynamicColour& colour) {
    const auto& palette = colour.palette(scheme);
    const auto tone = colour.getTone(scheme);
    const auto multiplier = colour.chromaMultiplier(scheme);
    if (isExactly(multiplier, 1.0))
        return palette.getHct(tone);

    const auto chroma = palette.chroma() * multiplier;
    if (isExactly(tone, 99.0) && Hct::isYellow(palette.hue()))
        return palettes::TonalPalette::fromHueAndChroma(palette.hue(), chroma).getHct(tone);
    return { palette.hue(), chroma, tone };
}

} // namespace

double ContrastCurve::get(double contrastLevel) const {
    if (contrastLevel <= -1.0)
        return low;
    if (contrastLevel < 0.0)
        return lerp(low, normal, contrastLevel + 1.0);
    if (contrastLevel < 0.5)
        return lerp(normal, medium, contrastLevel / 0.5);
    if (contrastLevel < 1.0)
        return lerp(medium, high, (contrastLevel - 0.5) / 0.5);
    return high;
}

DynamicColour::DynamicColour(DynamicColourOptions options)
    : m_name(std::move(options.name))
    , m_isBackground(options.isBackground)
    , m_base(std::move(options.base))
    , m_spec2025(std::move(options.spec2025))
    , m_spec2026(std::move(options.spec2026)) {
    validate(m_name, m_base);
    if (m_spec2025)
        validate(m_name, *m_spec2025);
    if (m_spec2026)
        validate(m_name, *m_spec2026);
}

const ColourSpec& DynamicColour::spec(const DynamicScheme& scheme) const {
    const auto version = scheme.specVersion();
    if (version >= SpecVersion::Spec2026 && m_spec2026)
        return *m_spec2026;
    if (version >= SpecVersion::Spec2025 && m_spec2025)
        return *m_spec2025;
    return m_base;
}

const palettes::TonalPalette& DynamicColour::palette(const DynamicScheme& scheme) const {
    return spec(scheme).palette(scheme);
}

double DynamicColour::tone(const DynamicScheme& scheme) const {
    const auto& current = spec(scheme);
    if (current.tone)
        return current.tone(scheme);
    const auto* bg = current.background ? current.background(scheme) : nullptr;
    return bg ? bg->getTone(scheme) : 50.0;
}

double DynamicColour::chromaMultiplier(const DynamicScheme& scheme) const {
    const auto& current = spec(scheme);
    return current.chromaMultiplier ? current.chromaMultiplier(scheme) : 1.0;
}

const DynamicColour* DynamicColour::background(const DynamicScheme& scheme) const {
    const auto& current = spec(scheme);
    return current.background ? current.background(scheme) : nullptr;
}

const DynamicColour* DynamicColour::secondBackground(const DynamicScheme& scheme) const {
    const auto& current = spec(scheme);
    return current.secondBackground ? current.secondBackground(scheme) : nullptr;
}

std::optional<ContrastCurve> DynamicColour::contrastCurve(const DynamicScheme& scheme) const {
    const auto& current = spec(scheme);
    return current.contrastCurve ? current.contrastCurve(scheme) : std::nullopt;
}

std::optional<ToneDeltaPair> DynamicColour::toneDeltaPair(const DynamicScheme& scheme) const {
    const auto& current = spec(scheme);
    return current.toneDeltaPair ? current.toneDeltaPair(scheme) : std::nullopt;
}

Argb DynamicColour::getArgb(const DynamicScheme& scheme) const {
    return getHct(scheme).toArgb();
}

Hct DynamicColour::getHct(const DynamicScheme& scheme) const {
    if (scheme.specVersion() == SpecVersion::Spec2021)
        return palette(scheme).getHct(getTone(scheme));
    return hctFor2025(scheme, *this);
}

double DynamicColour::getTone(const DynamicScheme& scheme) const {
    auto& cache = scheme.m_toneCache;
    if (const auto it = cache.find(this); it != cache.end())
        return it->second;

    const auto tone = scheme.specVersion() == SpecVersion::Spec2021 // format hack
                          ? toneFor2021(scheme, *this)
                          : toneFor2025(scheme, *this);
    cache.emplace(this, tone);
    return tone;
}

double DynamicColour::foregroundTone(double bgTone, double ratio) {
    const auto lighterTone = contrast::lighterUnsafe(bgTone, ratio);
    const auto darkerTone = contrast::darkerUnsafe(bgTone, ratio);
    const auto lighterRatio = contrast::ratioOfTones(lighterTone, bgTone);
    const auto darkerRatio = contrast::ratioOfTones(darkerTone, bgTone);

    if (tonePrefersLightForeground(bgTone)) {
        const auto negligibleDifference =
            std::abs(lighterRatio - darkerRatio) < 0.1 && lighterRatio < ratio && darkerRatio < ratio;
        return lighterRatio >= ratio || lighterRatio >= darkerRatio || negligibleDifference ? lighterTone : darkerTone;
    }
    return darkerRatio >= ratio || darkerRatio >= lighterRatio ? darkerTone : lighterTone;
}

bool DynamicColour::tonePrefersLightForeground(double tone) {
    return std::round(tone) < 60.0;
}

bool DynamicColour::toneAllowsLightForeground(double tone) {
    return std::round(tone) <= 49.0;
}

double DynamicColour::enableLightForeground(double tone) {
    if (tonePrefersLightForeground(tone) && !toneAllowsLightForeground(tone))
        return 49.0;
    return tone;
}

} // namespace m3colours::dynamiccolour
