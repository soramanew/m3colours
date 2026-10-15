#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "hct.hpp"
#include "palettes.hpp"

namespace m3colours::dynamiccolour {

enum class Variant : std::uint8_t {
    Monochrome,
    Neutral,
    TonalSpot,
    Vibrant,
    Expressive,
    Fidelity,
    Content,
    Rainbow,
    FruitSalad,
    Cmf,
};

enum class Platform : std::uint8_t {
    Phone,
    Watch,
};

enum class SpecVersion : std::uint8_t {
    Spec2021,
    Spec2025,
    Spec2026,
};

enum class TonePolarity : std::uint8_t {
    Darker,
    Lighter,
    Nearer,
    Farther,
    RelativeDarker,
    RelativeLighter,
};

enum class DeltaConstraint : std::uint8_t {
    Exact,
    Nearer,
    Farther,
};

struct ContrastCurve {
    constexpr ContrastCurve(double lowValue, double normalValue, double mediumValue, double highValue)
        : low(lowValue)
        , normal(normalValue)
        , medium(mediumValue)
        , high(highValue) {}

    [[nodiscard]] double get(double contrastLevel) const;

    double low;
    double normal;
    double medium;
    double high;
};

class DynamicColour;
class DynamicScheme;

struct ToneDeltaPair {
    ToneDeltaPair(const DynamicColour& first, const DynamicColour& second, double toneDelta, TonePolarity tonePolarity,
        bool keepTogether, DeltaConstraint deltaConstraint = DeltaConstraint::Exact)
        : roleA(first)
        , roleB(second)
        , delta(toneDelta)
        , polarity(tonePolarity)
        , stayTogether(keepTogether)
        , constraint(deltaConstraint) {}

    const DynamicColour& roleA;
    const DynamicColour& roleB;
    double delta;
    TonePolarity polarity;
    bool stayTogether;
    DeltaConstraint constraint;
};

struct ColourSpec {
    std::function<const palettes::TonalPalette&(const DynamicScheme&)> palette;
    std::function<double(const DynamicScheme&)> tone;
    std::function<double(const DynamicScheme&)> chromaMultiplier;
    std::function<const DynamicColour*(const DynamicScheme&)> background;
    std::function<const DynamicColour*(const DynamicScheme&)> secondBackground;
    std::function<std::optional<ContrastCurve>(const DynamicScheme&)> contrastCurve;
    std::function<std::optional<ToneDeltaPair>(const DynamicScheme&)> toneDeltaPair;
};

struct DynamicColourOptions {
    std::string name;
    bool isBackground = false;
    ColourSpec base;
    std::optional<ColourSpec> spec2025;
    std::optional<ColourSpec> spec2026;
};

class DynamicColour {
public:
    explicit DynamicColour(DynamicColourOptions options);

    // clang-format off
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] bool isBackground() const { return m_isBackground; }
    // clang-format on

    [[nodiscard]] const palettes::TonalPalette& palette(const DynamicScheme& scheme) const;
    [[nodiscard]] double tone(const DynamicScheme& scheme) const;
    [[nodiscard]] double chromaMultiplier(const DynamicScheme& scheme) const;
    [[nodiscard]] const DynamicColour* background(const DynamicScheme& scheme) const;
    [[nodiscard]] const DynamicColour* secondBackground(const DynamicScheme& scheme) const;
    [[nodiscard]] std::optional<ContrastCurve> contrastCurve(const DynamicScheme& scheme) const;
    [[nodiscard]] std::optional<ToneDeltaPair> toneDeltaPair(const DynamicScheme& scheme) const;

    [[nodiscard]] Argb getArgb(const DynamicScheme& scheme) const;
    [[nodiscard]] Hct getHct(const DynamicScheme& scheme) const;
    [[nodiscard]] double getTone(const DynamicScheme& scheme) const;

    static double foregroundTone(double bgTone, double ratio);
    static bool tonePrefersLightForeground(double tone);
    static bool toneAllowsLightForeground(double tone);
    static double enableLightForeground(double tone);

private:
    [[nodiscard]] const ColourSpec& spec(const DynamicScheme& scheme) const;

    std::string m_name;
    bool m_isBackground;
    ColourSpec m_base;
    std::optional<ColourSpec> m_spec2025;
    std::optional<ColourSpec> m_spec2026;
};

struct DynamicSchemeOptions {
    std::vector<Hct> sourceColourHcts;
    Variant variant = Variant::TonalSpot;
    double contrastLevel = 0.0;
    bool isDark = false;
    Platform platform = Platform::Phone;
    SpecVersion specVersion = SpecVersion::Spec2021;
    std::optional<palettes::TonalPalette> primaryPalette;
    std::optional<palettes::TonalPalette> secondaryPalette;
    std::optional<palettes::TonalPalette> tertiaryPalette;
    std::optional<palettes::TonalPalette> neutralPalette;
    std::optional<palettes::TonalPalette> neutralVariantPalette;
    std::optional<palettes::TonalPalette> errorPalette;
};

class DynamicScheme {
public:
    explicit DynamicScheme(DynamicSchemeOptions options);

    // clang-format off
    [[nodiscard]] const Hct& sourceColourHct() const { return m_sourceColourHcts.front(); }
    [[nodiscard]] const std::vector<Hct>& sourceColourHcts() const { return m_sourceColourHcts; }
    [[nodiscard]] Argb sourceColourArgb() const { return sourceColourHct().toArgb(); }
    [[nodiscard]] Variant variant() const { return m_variant; }
    [[nodiscard]] double contrastLevel() const { return m_contrastLevel; }
    [[nodiscard]] bool isDark() const { return m_isDark; }
    [[nodiscard]] Platform platform() const { return m_platform; }
    [[nodiscard]] SpecVersion specVersion() const { return m_specVersion; }
    [[nodiscard]] const palettes::TonalPalette& primaryPalette() const { return m_primaryPalette; }
    [[nodiscard]] const palettes::TonalPalette& secondaryPalette() const { return m_secondaryPalette; }
    [[nodiscard]] const palettes::TonalPalette& tertiaryPalette() const { return m_tertiaryPalette; }
    [[nodiscard]] const palettes::TonalPalette& neutralPalette() const { return m_neutralPalette; }
    [[nodiscard]] const palettes::TonalPalette& neutralVariantPalette() const { return m_neutralVariantPalette; }
    [[nodiscard]] const palettes::TonalPalette& errorPalette() const { return m_errorPalette; }
    // clang-format on

    [[nodiscard]] DynamicScheme asLightStandardContrast() const;

    static double getPiecewiseHue(
        const Hct& sourceColourHct, std::span<const double> hueBreakpoints, std::span<const double> hues);
    static double getRotatedHue(
        const Hct& sourceColourHct, std::span<const double> hueBreakpoints, std::span<const double> rotations);

    [[nodiscard]] Argb getArgb(const DynamicColour& colour) const;
    [[nodiscard]] Hct getHct(const DynamicColour& colour) const;

    [[nodiscard]] Argb primaryPaletteKeyColour() const;
    [[nodiscard]] Argb secondaryPaletteKeyColour() const;
    [[nodiscard]] Argb tertiaryPaletteKeyColour() const;
    [[nodiscard]] Argb neutralPaletteKeyColour() const;
    [[nodiscard]] Argb neutralVariantPaletteKeyColour() const;
    [[nodiscard]] Argb errorPaletteKeyColour() const;

    [[nodiscard]] Argb background() const;
    [[nodiscard]] Argb onBackground() const;
    [[nodiscard]] Argb surface() const;
    [[nodiscard]] Argb surfaceDim() const;
    [[nodiscard]] Argb surfaceBright() const;
    [[nodiscard]] Argb surfaceContainerLowest() const;
    [[nodiscard]] Argb surfaceContainerLow() const;
    [[nodiscard]] Argb surfaceContainer() const;
    [[nodiscard]] Argb surfaceContainerHigh() const;
    [[nodiscard]] Argb surfaceContainerHighest() const;
    [[nodiscard]] Argb onSurface() const;
    [[nodiscard]] Argb surfaceVariant() const;
    [[nodiscard]] Argb onSurfaceVariant() const;
    [[nodiscard]] Argb inverseSurface() const;
    [[nodiscard]] Argb inverseOnSurface() const;
    [[nodiscard]] Argb outline() const;
    [[nodiscard]] Argb outlineVariant() const;
    [[nodiscard]] Argb shadow() const;
    [[nodiscard]] Argb scrim() const;
    [[nodiscard]] Argb surfaceTint() const;

    [[nodiscard]] Argb primary() const;
    [[nodiscard]] std::optional<Argb> primaryDim() const;
    [[nodiscard]] Argb onPrimary() const;
    [[nodiscard]] Argb primaryContainer() const;
    [[nodiscard]] Argb onPrimaryContainer() const;
    [[nodiscard]] Argb primaryFixed() const;
    [[nodiscard]] Argb primaryFixedDim() const;
    [[nodiscard]] Argb onPrimaryFixed() const;
    [[nodiscard]] Argb onPrimaryFixedVariant() const;
    [[nodiscard]] Argb inversePrimary() const;

    [[nodiscard]] Argb secondary() const;
    [[nodiscard]] std::optional<Argb> secondaryDim() const;
    [[nodiscard]] Argb onSecondary() const;
    [[nodiscard]] Argb secondaryContainer() const;
    [[nodiscard]] Argb onSecondaryContainer() const;
    [[nodiscard]] Argb secondaryFixed() const;
    [[nodiscard]] Argb secondaryFixedDim() const;
    [[nodiscard]] Argb onSecondaryFixed() const;
    [[nodiscard]] Argb onSecondaryFixedVariant() const;

    [[nodiscard]] Argb tertiary() const;
    [[nodiscard]] std::optional<Argb> tertiaryDim() const;
    [[nodiscard]] Argb onTertiary() const;
    [[nodiscard]] Argb tertiaryContainer() const;
    [[nodiscard]] Argb onTertiaryContainer() const;
    [[nodiscard]] Argb tertiaryFixed() const;
    [[nodiscard]] Argb tertiaryFixedDim() const;
    [[nodiscard]] Argb onTertiaryFixed() const;
    [[nodiscard]] Argb onTertiaryFixedVariant() const;

    [[nodiscard]] Argb error() const;
    [[nodiscard]] std::optional<Argb> errorDim() const;
    [[nodiscard]] Argb onError() const;
    [[nodiscard]] Argb errorContainer() const;
    [[nodiscard]] Argb onErrorContainer() const;

private:
    friend class DynamicColour;

    std::vector<Hct> m_sourceColourHcts;
    Variant m_variant;
    double m_contrastLevel;
    bool m_isDark;
    Platform m_platform;
    SpecVersion m_specVersion;
    palettes::TonalPalette m_primaryPalette;
    palettes::TonalPalette m_secondaryPalette;
    palettes::TonalPalette m_tertiaryPalette;
    palettes::TonalPalette m_neutralPalette;
    palettes::TonalPalette m_neutralVariantPalette;
    palettes::TonalPalette m_errorPalette;
    mutable std::unordered_map<const DynamicColour*, double> m_toneCache;
};

namespace material {

const DynamicColour& highestSurface(const DynamicScheme& scheme);

const DynamicColour& primaryPaletteKeyColour();
const DynamicColour& secondaryPaletteKeyColour();
const DynamicColour& tertiaryPaletteKeyColour();
const DynamicColour& neutralPaletteKeyColour();
const DynamicColour& neutralVariantPaletteKeyColour();
const DynamicColour& errorPaletteKeyColour();

const DynamicColour& background();
const DynamicColour& onBackground();
const DynamicColour& surface();
const DynamicColour& surfaceDim();
const DynamicColour& surfaceBright();
const DynamicColour& surfaceContainerLowest();
const DynamicColour& surfaceContainerLow();
const DynamicColour& surfaceContainer();
const DynamicColour& surfaceContainerHigh();
const DynamicColour& surfaceContainerHighest();
const DynamicColour& onSurface();
const DynamicColour& surfaceVariant();
const DynamicColour& onSurfaceVariant();
const DynamicColour& inverseSurface();
const DynamicColour& inverseOnSurface();
const DynamicColour& outline();
const DynamicColour& outlineVariant();
const DynamicColour& shadow();
const DynamicColour& scrim();
const DynamicColour& surfaceTint();

const DynamicColour& primary();
const DynamicColour& primaryDim();
const DynamicColour& onPrimary();
const DynamicColour& primaryContainer();
const DynamicColour& onPrimaryContainer();
const DynamicColour& primaryFixed();
const DynamicColour& primaryFixedDim();
const DynamicColour& onPrimaryFixed();
const DynamicColour& onPrimaryFixedVariant();
const DynamicColour& inversePrimary();

const DynamicColour& secondary();
const DynamicColour& secondaryDim();
const DynamicColour& onSecondary();
const DynamicColour& secondaryContainer();
const DynamicColour& onSecondaryContainer();
const DynamicColour& secondaryFixed();
const DynamicColour& secondaryFixedDim();
const DynamicColour& onSecondaryFixed();
const DynamicColour& onSecondaryFixedVariant();

const DynamicColour& tertiary();
const DynamicColour& tertiaryDim();
const DynamicColour& onTertiary();
const DynamicColour& tertiaryContainer();
const DynamicColour& onTertiaryContainer();
const DynamicColour& tertiaryFixed();
const DynamicColour& tertiaryFixedDim();
const DynamicColour& onTertiaryFixed();
const DynamicColour& onTertiaryFixedVariant();

const DynamicColour& error();
const DynamicColour& errorDim();
const DynamicColour& onError();
const DynamicColour& errorContainer();
const DynamicColour& onErrorContainer();

std::span<const DynamicColour* const> allColours();

} // namespace material

} // namespace m3colours::dynamiccolour
