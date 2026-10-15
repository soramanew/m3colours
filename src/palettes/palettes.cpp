#include "m3colours/palettes.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>

namespace m3colours::palettes {

namespace {

hct::internal::Jch jchFromArgb(Argb argb) {
    return hct::internal::jchFromXyz(hct::internal::xyzFromArgb(argb), hct::k_defaultViewingConditions);
}

std::uint8_t averageComponent(std::uint8_t a, std::uint8_t b) {
    return static_cast<std::uint8_t>((a + b + 1) / 2);
}

Argb averageArgb(Argb a, Argb b) {
    return Argb{ averageComponent(a.red(), b.red()), averageComponent(a.green(), b.green()),
        averageComponent(a.blue(), b.blue()) };
}

// The key colour is the first tone, searching outwards from T50, that can reach the requested chroma.
hct::Hct createKeyColour(double hue, double requestedChroma) {
    constexpr auto k_pivotTone = 50;
    constexpr auto k_toneStepSize = 1;
    constexpr auto k_epsilon = 0.01;
    constexpr auto k_maxChromaValue = 200.0;

    std::array<double, 101> chromaCache{};
    chromaCache.fill(-1.0);
    const auto maxChroma = [&](int tone) {
        auto& cached = chromaCache[static_cast<std::size_t>(tone)];
        if (cached < 0.0)
            cached = jchFromArgb(hct::solveToArgb(hue, k_maxChromaValue, tone)).chroma;
        return cached;
    };

    auto lowerTone = 0;
    auto upperTone = 100;
    while (lowerTone < upperTone) {
        const auto midTone = (lowerTone + upperTone) / 2;
        const auto isAscending = maxChroma(midTone) < maxChroma(midTone + k_toneStepSize);
        const auto sufficientChroma = maxChroma(midTone) >= requestedChroma - k_epsilon;

        if (sufficientChroma) {
            if (std::abs(lowerTone - k_pivotTone) < std::abs(upperTone - k_pivotTone)) {
                upperTone = midTone;
            } else {
                if (lowerTone == midTone)
                    return { hue, requestedChroma, static_cast<double>(lowerTone) };
                lowerTone = midTone;
            }
        } else if (isAscending) {
            lowerTone = midTone + k_toneStepSize;
        } else {
            upperTone = midTone;
        }
    }

    return { hue, requestedChroma, static_cast<double>(lowerTone) };
}

bool isTone99(double tone) {
    return !(tone < 99.0) && !(tone > 99.0);
}

} // namespace

TonalPalette TonalPalette::fromArgb(Argb argb) {
    return fromHct(hct::Hct{ argb });
}

TonalPalette TonalPalette::fromHct(const hct::Hct& hct) {
    return { hct.hue(), hct.chroma(), hct };
}

TonalPalette TonalPalette::fromHueAndChroma(double hue, double chroma) {
    return { hue, chroma, createKeyColour(hue, chroma) };
}

TonalPalette::TonalPalette(double hue, double chroma, hct::Hct keyColour)
    : m_hue(hue)
    , m_chroma(chroma)
    , m_keyColour(keyColour) {}

Argb TonalPalette::tone(double tone) const {
    if (const auto it = m_cache.find(tone); it != m_cache.end())
        return it->second;

    const auto argb = isTone99(tone) && hct::Hct::isYellow(m_hue) // format hack
                          ? averageArgb(this->tone(98.0), this->tone(100.0))
                          : hct::solveToArgb(m_hue, m_chroma, tone);
    m_cache.emplace(tone, argb);
    return argb;
}

hct::Hct TonalPalette::getHct(double tone) const {
    return hct::Hct{ this->tone(tone) };
}

CorePalette CorePalette::of(Argb argb) {
    const auto jch = jchFromArgb(argb);
    return { jch.hue, jch.chroma, false };
}

CorePalette CorePalette::contentOf(Argb argb) {
    const auto jch = jchFromArgb(argb);
    return { jch.hue, jch.chroma, true };
}

CorePalette CorePalette::fromColours(const CorePaletteColours& colours) {
    auto palette = of(colours.primary);
    if (colours.secondary)
        palette.a2 = of(*colours.secondary).a1;
    if (colours.tertiary)
        palette.a3 = of(*colours.tertiary).a1;
    if (colours.error)
        palette.error = of(*colours.error).a1;
    if (colours.neutral)
        palette.n1 = of(*colours.neutral).n1;
    if (colours.neutralVariant)
        palette.n2 = of(*colours.neutralVariant).n2;
    return palette;
}

CorePalette CorePalette::contentFromColours(const CorePaletteColours& colours) {
    auto palette = contentOf(colours.primary);
    if (colours.secondary)
        palette.a2 = contentOf(*colours.secondary).a1;
    if (colours.tertiary)
        palette.a3 = contentOf(*colours.tertiary).a1;
    if (colours.error)
        palette.error = contentOf(*colours.error).a1;
    if (colours.neutral)
        palette.n1 = contentOf(*colours.neutral).n1;
    if (colours.neutralVariant)
        palette.n2 = contentOf(*colours.neutralVariant).n2;
    return palette;
}

CorePalette::CorePalette(double hue, double chroma, bool isContent)
    : a1(TonalPalette::fromHueAndChroma(hue, isContent ? chroma : std::max(48.0, chroma)))
    , a2(TonalPalette::fromHueAndChroma(hue, isContent ? chroma / 3.0 : 16.0))
    , a3(TonalPalette::fromHueAndChroma(hue + 60.0, isContent ? chroma / 2.0 : 24.0))
    , n1(TonalPalette::fromHueAndChroma(hue, isContent ? std::min(chroma / 12.0, 4.0) : 4.0))
    , n2(TonalPalette::fromHueAndChroma(hue, isContent ? std::min(chroma / 6.0, 8.0) : 8.0))
    , error(TonalPalette::fromHueAndChroma(25.0, 84.0)) {}

} // namespace m3colours::palettes
