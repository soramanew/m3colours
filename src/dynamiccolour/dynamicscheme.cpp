#include <algorithm>
#include <stdexcept>

#include "m3colours/dislike.hpp"
#include "m3colours/dynamiccolour.hpp"
#include "m3colours/temperature.hpp"

namespace m3colours::dynamiccolour {

namespace {

using palettes::TonalPalette;

SpecVersion fallbackSpecVersion(SpecVersion specVersion, Variant variant) {
    switch (variant) {
    case Variant::Cmf:
        return specVersion;
    case Variant::Expressive:
    case Variant::Vibrant:
    case Variant::TonalSpot:
    case Variant::Neutral:
        return specVersion == SpecVersion::Spec2026 ? SpecVersion::Spec2025 : specVersion;
    default:
        return SpecVersion::Spec2021;
    }
}

std::vector<Hct> requireSourceColours(std::vector<Hct> sourceColourHcts) {
    if (sourceColourHcts.empty())
        throw std::invalid_argument("sourceColourHcts cannot be empty");
    return sourceColourHcts;
}

double neutralChroma2025(
    const Hct& source, bool phone, double phoneBlue, double phoneOther, double watchBlue, double watchOther) {
    const auto blue = Hct::isBlue(source.hue());
    if (phone)
        return blue ? phoneBlue : phoneOther;
    return blue ? watchBlue : watchOther;
}

[[noreturn]] void unsupportedVariant() {
    throw std::invalid_argument("Unsupported variant");
}

std::optional<TonalPalette> primaryPalette2025For(Variant variant, const Hct& source, bool isDark, Platform platform) {
    const auto phone = platform == Platform::Phone;
    switch (variant) {
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), neutralChroma2025(source, phone, 12.0, 8.0, 16.0, 12.0));
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), phone && isDark ? 26.0 : 32.0);
    case Variant::Expressive:
        if (!phone)
            return TonalPalette::fromHueAndChroma(source.hue(), 40.0);
        return TonalPalette::fromHueAndChroma(source.hue(), isDark ? 36.0 : 48.0);
    case Variant::Vibrant:
        return TonalPalette::fromHueAndChroma(source.hue(), phone ? 74.0 : 56.0);
    default:
        return std::nullopt;
    }
}

TonalPalette primaryPaletteFor(SpecVersion spec, Variant variant, const Hct& source, bool isDark, Platform platform) {
    if (spec == SpecVersion::Spec2025) {
        if (auto palette = primaryPalette2025For(variant, source, isDark, platform))
            return std::move(*palette);
    }

    switch (variant) {
    case Variant::Content:
    case Variant::Fidelity:
        return TonalPalette::fromHueAndChroma(source.hue(), source.chroma());
    case Variant::FruitSalad:
        return TonalPalette::fromHueAndChroma(sanitiseDegrees(source.hue() - 50.0), 48.0);
    case Variant::Monochrome:
        return TonalPalette::fromHueAndChroma(source.hue(), 0.0);
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), 12.0);
    case Variant::Rainbow:
        return TonalPalette::fromHueAndChroma(source.hue(), 48.0);
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), 36.0);
    case Variant::Expressive:
        return TonalPalette::fromHueAndChroma(sanitiseDegrees(source.hue() + 240.0), 40.0);
    case Variant::Vibrant:
        return TonalPalette::fromHueAndChroma(source.hue(), 200.0);
    default:
        unsupportedVariant();
    }
}

std::optional<TonalPalette> secondaryPalette2025For(
    Variant variant, const Hct& source, bool isDark, Platform platform) {
    const auto phone = platform == Platform::Phone;
    switch (variant) {
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), neutralChroma2025(source, phone, 6.0, 4.0, 10.0, 6.0));
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), 16.0);
    case Variant::Expressive: {
        constexpr double k_breakpoints[]{ 0, 105, 140, 204, 253, 278, 300, 333, 360 };
        constexpr double k_rotations[]{ -160, 155, -100, 96, -96, -156, -165, -160 };
        return TonalPalette::fromHueAndChroma(
            DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), phone && isDark ? 16.0 : 24.0);
    }
    case Variant::Vibrant: {
        constexpr double k_breakpoints[]{ 0, 38, 105, 140, 333, 360 };
        constexpr double k_rotations[]{ -14, 10, -14, 10, -14 };
        return TonalPalette::fromHueAndChroma(
            DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), phone ? 56.0 : 36.0);
    }
    default:
        return std::nullopt;
    }
}

TonalPalette secondaryPaletteFor(SpecVersion spec, Variant variant, const Hct& source, bool isDark, Platform platform) {
    if (spec == SpecVersion::Spec2025) {
        if (auto palette = secondaryPalette2025For(variant, source, isDark, platform))
            return std::move(*palette);
    }

    switch (variant) {
    case Variant::Content:
    case Variant::Fidelity:
        return TonalPalette::fromHueAndChroma(source.hue(), std::max(source.chroma() - 32.0, source.chroma() * 0.5));
    case Variant::FruitSalad:
        return TonalPalette::fromHueAndChroma(sanitiseDegrees(source.hue() - 50.0), 36.0);
    case Variant::Monochrome:
        return TonalPalette::fromHueAndChroma(source.hue(), 0.0);
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), 8.0);
    case Variant::Rainbow:
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), 16.0);
    case Variant::Expressive: {
        constexpr double k_breakpoints[]{ 0, 21, 51, 121, 151, 191, 271, 321, 360 };
        constexpr double k_rotations[]{ 45, 95, 45, 20, 45, 90, 45, 45, 45 };
        return TonalPalette::fromHueAndChroma(DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), 24.0);
    }
    case Variant::Vibrant: {
        constexpr double k_breakpoints[]{ 0, 41, 61, 101, 131, 181, 251, 301, 360 };
        constexpr double k_rotations[]{ 18, 15, 10, 12, 15, 18, 15, 12, 12 };
        return TonalPalette::fromHueAndChroma(DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), 24.0);
    }
    default:
        unsupportedVariant();
    }
}

std::optional<TonalPalette> tertiaryPalette2025For(Variant variant, const Hct& source, Platform platform) {
    const auto phone = platform == Platform::Phone;
    switch (variant) {
    case Variant::Neutral: {
        constexpr double k_breakpoints[]{ 0, 38, 105, 161, 204, 278, 333, 360 };
        constexpr double k_rotations[]{ -32, 26, 10, -39, 24, -15, -32 };
        return TonalPalette::fromHueAndChroma(
            DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), phone ? 20.0 : 36.0);
    }
    case Variant::TonalSpot: {
        constexpr double k_breakpoints[]{ 0, 20, 71, 161, 333, 360 };
        constexpr double k_rotations[]{ -40, 48, -32, 40, -32 };
        return TonalPalette::fromHueAndChroma(
            DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), phone ? 28.0 : 32.0);
    }
    case Variant::Expressive: {
        constexpr double k_breakpoints[]{ 0, 105, 140, 204, 253, 278, 300, 333, 360 };
        constexpr double k_rotations[]{ -165, 160, -105, 101, -101, -160, -170, -165 };
        return TonalPalette::fromHueAndChroma(DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), 48.0);
    }
    case Variant::Vibrant: {
        constexpr double k_breakpoints[]{ 0, 38, 71, 105, 140, 161, 253, 333, 360 };
        constexpr double k_rotations[]{ -72, 35, 24, -24, 62, 50, 62, -72 };
        return TonalPalette::fromHueAndChroma(DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), 56.0);
    }
    default:
        return std::nullopt;
    }
}

TonalPalette tertiaryPaletteFor(SpecVersion spec, Variant variant, const Hct& source, Platform platform) {
    if (spec == SpecVersion::Spec2025) {
        if (auto palette = tertiaryPalette2025For(variant, source, platform))
            return std::move(*palette);
    }

    switch (variant) {
    case Variant::Content:
        return TonalPalette::fromHct(dislike::fixIfDisliked(temperature::TemperatureCache(source).analogous(3, 6)[2]));
    case Variant::Fidelity:
        return TonalPalette::fromHct(dislike::fixIfDisliked(temperature::TemperatureCache(source).complement()));
    case Variant::FruitSalad:
        return TonalPalette::fromHueAndChroma(source.hue(), 36.0);
    case Variant::Monochrome:
        return TonalPalette::fromHueAndChroma(source.hue(), 0.0);
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), 16.0);
    case Variant::Rainbow:
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(sanitiseDegrees(source.hue() + 60.0), 24.0);
    case Variant::Expressive: {
        constexpr double k_breakpoints[]{ 0, 21, 51, 121, 151, 191, 271, 321, 360 };
        constexpr double k_rotations[]{ 120, 120, 20, 45, 20, 15, 20, 120, 120 };
        return TonalPalette::fromHueAndChroma(DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), 32.0);
    }
    case Variant::Vibrant: {
        constexpr double k_breakpoints[]{ 0, 41, 61, 101, 131, 181, 251, 301, 360 };
        constexpr double k_rotations[]{ 35, 30, 20, 25, 30, 35, 30, 25, 25 };
        return TonalPalette::fromHueAndChroma(DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations), 32.0);
    }
    default:
        unsupportedVariant();
    }
}

double expressiveNeutralHue(const Hct& source) {
    constexpr double k_breakpoints[]{ 0, 71, 124, 253, 278, 300, 360 };
    constexpr double k_rotations[]{ 10, 0, 10, 0, 10, 0 };
    return DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations);
}

double expressiveNeutralChroma(const Hct& source, bool isDark, Platform platform) {
    if (platform != Platform::Phone)
        return 12.0;
    if (!isDark)
        return 18.0;
    return Hct::isYellow(expressiveNeutralHue(source)) ? 6.0 : 14.0;
}

double vibrantNeutralHue(const Hct& source) {
    constexpr double k_breakpoints[]{ 0, 38, 105, 140, 333, 360 };
    constexpr double k_rotations[]{ -14, 10, -14, 10, -14 };
    return DynamicScheme::getRotatedHue(source, k_breakpoints, k_rotations);
}

double vibrantNeutralChroma(const Hct& source, Platform platform) {
    if (platform == Platform::Phone)
        return 28.0;
    return Hct::isBlue(vibrantNeutralHue(source)) ? 28.0 : 20.0;
}

std::optional<TonalPalette> neutralPalette2025For(Variant variant, const Hct& source, bool isDark, Platform platform) {
    const auto phone = platform == Platform::Phone;
    switch (variant) {
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), phone ? 1.4 : 6.0);
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), phone ? 5.0 : 10.0);
    case Variant::Expressive:
        return TonalPalette::fromHueAndChroma(
            expressiveNeutralHue(source), expressiveNeutralChroma(source, isDark, platform));
    case Variant::Vibrant:
        return TonalPalette::fromHueAndChroma(vibrantNeutralHue(source), vibrantNeutralChroma(source, platform));
    default:
        return std::nullopt;
    }
}

TonalPalette neutralPaletteFor(SpecVersion spec, Variant variant, const Hct& source, bool isDark, Platform platform) {
    if (spec == SpecVersion::Spec2025) {
        if (auto palette = neutralPalette2025For(variant, source, isDark, platform))
            return std::move(*palette);
    }

    switch (variant) {
    case Variant::Content:
    case Variant::Fidelity:
        return TonalPalette::fromHueAndChroma(source.hue(), source.chroma() / 8.0);
    case Variant::FruitSalad:
        return TonalPalette::fromHueAndChroma(source.hue(), 10.0);
    case Variant::Monochrome:
    case Variant::Rainbow:
        return TonalPalette::fromHueAndChroma(source.hue(), 0.0);
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), 2.0);
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), 6.0);
    case Variant::Expressive:
        return TonalPalette::fromHueAndChroma(sanitiseDegrees(source.hue() + 15.0), 8.0);
    case Variant::Vibrant:
        return TonalPalette::fromHueAndChroma(source.hue(), 10.0);
    default:
        unsupportedVariant();
    }
}

std::optional<TonalPalette> neutralVariantPalette2025For(
    Variant variant, const Hct& source, bool isDark, Platform platform) {
    const auto phone = platform == Platform::Phone;
    switch (variant) {
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), (phone ? 1.4 : 6.0) * 2.2);
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), (phone ? 5.0 : 10.0) * 1.7);
    case Variant::Expressive: {
        const auto hue = expressiveNeutralHue(source);
        const auto chroma = expressiveNeutralChroma(source, isDark, platform);
        return TonalPalette::fromHueAndChroma(hue, chroma * (hue >= 105.0 && hue < 125.0 ? 1.6 : 2.3));
    }
    case Variant::Vibrant:
        return TonalPalette::fromHueAndChroma(vibrantNeutralHue(source), vibrantNeutralChroma(source, platform) * 1.29);
    default:
        return std::nullopt;
    }
}

TonalPalette neutralVariantPaletteFor(
    SpecVersion spec, Variant variant, const Hct& source, bool isDark, Platform platform) {
    if (spec == SpecVersion::Spec2025) {
        if (auto palette = neutralVariantPalette2025For(variant, source, isDark, platform))
            return std::move(*palette);
    }

    switch (variant) {
    case Variant::Content:
    case Variant::Fidelity:
        return TonalPalette::fromHueAndChroma(source.hue(), (source.chroma() / 8.0) + 4.0);
    case Variant::FruitSalad:
        return TonalPalette::fromHueAndChroma(source.hue(), 16.0);
    case Variant::Monochrome:
    case Variant::Rainbow:
        return TonalPalette::fromHueAndChroma(source.hue(), 0.0);
    case Variant::Neutral:
        return TonalPalette::fromHueAndChroma(source.hue(), 2.0);
    case Variant::TonalSpot:
        return TonalPalette::fromHueAndChroma(source.hue(), 8.0);
    case Variant::Expressive:
        return TonalPalette::fromHueAndChroma(sanitiseDegrees(source.hue() + 15.0), 12.0);
    case Variant::Vibrant:
        return TonalPalette::fromHueAndChroma(source.hue(), 12.0);
    default:
        unsupportedVariant();
    }
}

TonalPalette errorPaletteFor(SpecVersion spec, Variant variant, const Hct& source, Platform platform) {
    const auto phone = platform == Platform::Phone;
    if (spec == SpecVersion::Spec2025) {
        constexpr double k_breakpoints[]{ 0, 3, 13, 23, 33, 43, 153, 273, 360 };
        constexpr double k_hues[]{ 12, 22, 32, 12, 22, 32, 22, 12 };
        const auto errorHue = DynamicScheme::getPiecewiseHue(source, k_breakpoints, k_hues);
        switch (variant) {
        case Variant::Neutral:
            return TonalPalette::fromHueAndChroma(errorHue, phone ? 50.0 : 40.0);
        case Variant::TonalSpot:
            return TonalPalette::fromHueAndChroma(errorHue, phone ? 60.0 : 48.0);
        case Variant::Expressive:
            return TonalPalette::fromHueAndChroma(errorHue, phone ? 64.0 : 48.0);
        case Variant::Vibrant:
            return TonalPalette::fromHueAndChroma(errorHue, phone ? 80.0 : 60.0);
        default:
            break;
        }
    }
    return TonalPalette::fromHueAndChroma(25.0, 84.0);
}

} // namespace

DynamicScheme::DynamicScheme(DynamicSchemeOptions options)
    : m_sourceColourHcts(requireSourceColours(std::move(options.sourceColourHcts)))
    , m_variant(options.variant)
    , m_contrastLevel(options.contrastLevel)
    , m_isDark(options.isDark)
    , m_platform(options.platform)
    , m_specVersion(fallbackSpecVersion(options.specVersion, options.variant))
    , m_primaryPalette(options.primaryPalette
                           ? std::move(*options.primaryPalette)
                           : primaryPaletteFor(m_specVersion, m_variant, sourceColourHct(), m_isDark, m_platform))
    , m_secondaryPalette(options.secondaryPalette
                             ? std::move(*options.secondaryPalette)
                             : secondaryPaletteFor(m_specVersion, m_variant, sourceColourHct(), m_isDark, m_platform))
    , m_tertiaryPalette(options.tertiaryPalette
                            ? std::move(*options.tertiaryPalette)
                            : tertiaryPaletteFor(m_specVersion, m_variant, sourceColourHct(), m_platform))
    , m_neutralPalette(options.neutralPalette
                           ? std::move(*options.neutralPalette)
                           : neutralPaletteFor(m_specVersion, m_variant, sourceColourHct(), m_isDark, m_platform))
    , m_neutralVariantPalette(options.neutralVariantPalette ? std::move(*options.neutralVariantPalette)
                                                            : neutralVariantPaletteFor(m_specVersion, m_variant,
                                                                  sourceColourHct(), m_isDark, m_platform))
    , m_errorPalette(options.errorPalette ? std::move(*options.errorPalette)
                                          : errorPaletteFor(m_specVersion, m_variant, sourceColourHct(), m_platform)) {}

DynamicScheme DynamicScheme::asLightStandardContrast() const {
    auto copy = *this;
    copy.m_isDark = false;
    copy.m_contrastLevel = 0.0;
    copy.m_toneCache.clear();
    return copy;
}

double DynamicScheme::getPiecewiseHue(
    const Hct& sourceColourHct, std::span<const double> hueBreakpoints, std::span<const double> hues) {
    const auto sourceHue = sourceColourHct.hue();
    if (hueBreakpoints.empty())
        return sourceHue;
    const auto size = std::min(hueBreakpoints.size() - 1, hues.size());
    for (std::size_t i = 0; i < size; i++) {
        if (sourceHue >= hueBreakpoints[i] && sourceHue < hueBreakpoints[i + 1])
            return sanitiseDegrees(hues[i]);
    }
    return sourceHue;
}

double DynamicScheme::getRotatedHue(
    const Hct& sourceColourHct, std::span<const double> hueBreakpoints, std::span<const double> rotations) {
    const auto hasRotations = hueBreakpoints.size() >= 2 && !rotations.empty();
    const auto rotation = hasRotations ? getPiecewiseHue(sourceColourHct, hueBreakpoints, rotations) : 0.0;
    return sanitiseDegrees(sourceColourHct.hue() + rotation);
}

Argb DynamicScheme::getArgb(const DynamicColour& colour) const {
    return colour.getArgb(*this);
}

Hct DynamicScheme::getHct(const DynamicColour& colour) const {
    return colour.getHct(*this);
}

// clang-format off
Argb DynamicScheme::primaryPaletteKeyColour() const { return getArgb(material::primaryPaletteKeyColour()); }
Argb DynamicScheme::secondaryPaletteKeyColour() const { return getArgb(material::secondaryPaletteKeyColour()); }
Argb DynamicScheme::tertiaryPaletteKeyColour() const { return getArgb(material::tertiaryPaletteKeyColour()); }
Argb DynamicScheme::neutralPaletteKeyColour() const { return getArgb(material::neutralPaletteKeyColour()); }
Argb DynamicScheme::neutralVariantPaletteKeyColour() const { return getArgb(material::neutralVariantPaletteKeyColour()); }
Argb DynamicScheme::errorPaletteKeyColour() const { return getArgb(material::errorPaletteKeyColour()); }

Argb DynamicScheme::background() const { return getArgb(material::background()); }
Argb DynamicScheme::onBackground() const { return getArgb(material::onBackground()); }
Argb DynamicScheme::surface() const { return getArgb(material::surface()); }
Argb DynamicScheme::surfaceDim() const { return getArgb(material::surfaceDim()); }
Argb DynamicScheme::surfaceBright() const { return getArgb(material::surfaceBright()); }
Argb DynamicScheme::surfaceContainerLowest() const { return getArgb(material::surfaceContainerLowest()); }
Argb DynamicScheme::surfaceContainerLow() const { return getArgb(material::surfaceContainerLow()); }
Argb DynamicScheme::surfaceContainer() const { return getArgb(material::surfaceContainer()); }
Argb DynamicScheme::surfaceContainerHigh() const { return getArgb(material::surfaceContainerHigh()); }
Argb DynamicScheme::surfaceContainerHighest() const { return getArgb(material::surfaceContainerHighest()); }
Argb DynamicScheme::onSurface() const { return getArgb(material::onSurface()); }
Argb DynamicScheme::surfaceVariant() const { return getArgb(material::surfaceVariant()); }
Argb DynamicScheme::onSurfaceVariant() const { return getArgb(material::onSurfaceVariant()); }
Argb DynamicScheme::inverseSurface() const { return getArgb(material::inverseSurface()); }
Argb DynamicScheme::inverseOnSurface() const { return getArgb(material::inverseOnSurface()); }
Argb DynamicScheme::outline() const { return getArgb(material::outline()); }
Argb DynamicScheme::outlineVariant() const { return getArgb(material::outlineVariant()); }
Argb DynamicScheme::shadow() const { return getArgb(material::shadow()); }
Argb DynamicScheme::scrim() const { return getArgb(material::scrim()); }
Argb DynamicScheme::surfaceTint() const { return getArgb(material::surfaceTint()); }

Argb DynamicScheme::primary() const { return getArgb(material::primary()); }
Argb DynamicScheme::onPrimary() const { return getArgb(material::onPrimary()); }
Argb DynamicScheme::primaryContainer() const { return getArgb(material::primaryContainer()); }
Argb DynamicScheme::onPrimaryContainer() const { return getArgb(material::onPrimaryContainer()); }
Argb DynamicScheme::primaryFixed() const { return getArgb(material::primaryFixed()); }
Argb DynamicScheme::primaryFixedDim() const { return getArgb(material::primaryFixedDim()); }
Argb DynamicScheme::onPrimaryFixed() const { return getArgb(material::onPrimaryFixed()); }
Argb DynamicScheme::onPrimaryFixedVariant() const { return getArgb(material::onPrimaryFixedVariant()); }
Argb DynamicScheme::inversePrimary() const { return getArgb(material::inversePrimary()); }

Argb DynamicScheme::secondary() const { return getArgb(material::secondary()); }
Argb DynamicScheme::onSecondary() const { return getArgb(material::onSecondary()); }
Argb DynamicScheme::secondaryContainer() const { return getArgb(material::secondaryContainer()); }
Argb DynamicScheme::onSecondaryContainer() const { return getArgb(material::onSecondaryContainer()); }
Argb DynamicScheme::secondaryFixed() const { return getArgb(material::secondaryFixed()); }
Argb DynamicScheme::secondaryFixedDim() const { return getArgb(material::secondaryFixedDim()); }
Argb DynamicScheme::onSecondaryFixed() const { return getArgb(material::onSecondaryFixed()); }
Argb DynamicScheme::onSecondaryFixedVariant() const { return getArgb(material::onSecondaryFixedVariant()); }

Argb DynamicScheme::tertiary() const { return getArgb(material::tertiary()); }
Argb DynamicScheme::onTertiary() const { return getArgb(material::onTertiary()); }
Argb DynamicScheme::tertiaryContainer() const { return getArgb(material::tertiaryContainer()); }
Argb DynamicScheme::onTertiaryContainer() const { return getArgb(material::onTertiaryContainer()); }
Argb DynamicScheme::tertiaryFixed() const { return getArgb(material::tertiaryFixed()); }
Argb DynamicScheme::tertiaryFixedDim() const { return getArgb(material::tertiaryFixedDim()); }
Argb DynamicScheme::onTertiaryFixed() const { return getArgb(material::onTertiaryFixed()); }
Argb DynamicScheme::onTertiaryFixedVariant() const { return getArgb(material::onTertiaryFixedVariant()); }

Argb DynamicScheme::error() const { return getArgb(material::error()); }
Argb DynamicScheme::onError() const { return getArgb(material::onError()); }
Argb DynamicScheme::errorContainer() const { return getArgb(material::errorContainer()); }
Argb DynamicScheme::onErrorContainer() const { return getArgb(material::onErrorContainer()); }
// clang-format on

std::optional<Argb> DynamicScheme::primaryDim() const {
    if (m_specVersion == SpecVersion::Spec2021)
        return std::nullopt;
    return getArgb(material::primaryDim());
}

std::optional<Argb> DynamicScheme::secondaryDim() const {
    if (m_specVersion == SpecVersion::Spec2021)
        return std::nullopt;
    return getArgb(material::secondaryDim());
}

std::optional<Argb> DynamicScheme::tertiaryDim() const {
    if (m_specVersion == SpecVersion::Spec2021)
        return std::nullopt;
    return getArgb(material::tertiaryDim());
}

std::optional<Argb> DynamicScheme::errorDim() const {
    if (m_specVersion == SpecVersion::Spec2021)
        return std::nullopt;
    return getArgb(material::errorDim());
}

} // namespace m3colours::dynamiccolour
