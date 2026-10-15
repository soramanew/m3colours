#pragma once

#include <utility>
#include <vector>

#include "common.hpp"
#include "dynamiccolour.hpp"
#include "hct.hpp"
#include "palettes.hpp"

namespace m3colours::scheme {

using dynamiccolour::DynamicScheme;
using dynamiccolour::DynamicSchemeOptions;
using dynamiccolour::Platform;
using dynamiccolour::SpecVersion;
using dynamiccolour::Variant;

struct Scheme {
    static Scheme light(Argb argb);
    static Scheme dark(Argb argb);
    static Scheme lightContent(Argb argb);
    static Scheme darkContent(Argb argb);
    static Scheme lightFromCorePalette(const palettes::CorePalette& core);
    static Scheme darkFromCorePalette(const palettes::CorePalette& core);

    Argb primary;
    Argb onPrimary;
    Argb primaryContainer;
    Argb onPrimaryContainer;
    Argb secondary;
    Argb onSecondary;
    Argb secondaryContainer;
    Argb onSecondaryContainer;
    Argb tertiary;
    Argb onTertiary;
    Argb tertiaryContainer;
    Argb onTertiaryContainer;
    Argb error;
    Argb onError;
    Argb errorContainer;
    Argb onErrorContainer;
    Argb background;
    Argb onBackground;
    Argb surface;
    Argb onSurface;
    Argb surfaceVariant;
    Argb onSurfaceVariant;
    Argb outline;
    Argb outlineVariant;
    Argb shadow;
    Argb scrim;
    Argb inverseSurface;
    Argb inverseOnSurface;
    Argb inversePrimary;
};

struct SchemeAndroid {
    static SchemeAndroid light(Argb argb);
    static SchemeAndroid dark(Argb argb);
    static SchemeAndroid lightContent(Argb argb);
    static SchemeAndroid darkContent(Argb argb);
    static SchemeAndroid lightFromCorePalette(const palettes::CorePalette& core);
    static SchemeAndroid darkFromCorePalette(const palettes::CorePalette& core);

    Argb colourAccentPrimary;
    Argb colourAccentPrimaryVariant;
    Argb colourAccentSecondary;
    Argb colourAccentSecondaryVariant;
    Argb colourAccentTertiary;
    Argb colourAccentTertiaryVariant;
    Argb textColourPrimary;
    Argb textColourSecondary;
    Argb textColourTertiary;
    Argb textColourPrimaryInverse;
    Argb textColourSecondaryInverse;
    Argb textColourTertiaryInverse;
    Argb colourBackground;
    Argb colourBackgroundFloating;
    Argb colourSurface;
    Argb colourSurfaceVariant;
    Argb colourSurfaceHighlight;
    Argb surfaceHeader;
    Argb underSurface;
    Argb offState;
    Argb accentSurface;
    Argb textPrimaryOnAccent;
    Argb textSecondaryOnAccent;
    Argb volumeBackground;
    Argb scrim;
};

template <Variant V> class VariantScheme : public DynamicScheme {
public:
    VariantScheme(std::vector<Hct> sourceColourHcts, bool isDark, double contrastLevel,
        SpecVersion specVersion = SpecVersion::Spec2021, Platform platform = Platform::Phone)
        : DynamicScheme({
              .sourceColourHcts = std::move(sourceColourHcts),
              .variant = V,
              .contrastLevel = contrastLevel,
              .isDark = isDark,
              .platform = platform,
              .specVersion = specVersion,
          }) {}

    VariantScheme(const Hct& sourceColourHct, bool isDark, double contrastLevel,
        SpecVersion specVersion = SpecVersion::Spec2021, Platform platform = Platform::Phone)
        : VariantScheme(std::vector{ sourceColourHct }, isDark, contrastLevel, specVersion, platform) {}
};

using SchemeMonochrome = VariantScheme<Variant::Monochrome>;
using SchemeNeutral = VariantScheme<Variant::Neutral>;
using SchemeTonalSpot = VariantScheme<Variant::TonalSpot>;
using SchemeVibrant = VariantScheme<Variant::Vibrant>;
using SchemeExpressive = VariantScheme<Variant::Expressive>;
using SchemeFidelity = VariantScheme<Variant::Fidelity>;
using SchemeContent = VariantScheme<Variant::Content>;
using SchemeRainbow = VariantScheme<Variant::Rainbow>;
using SchemeFruitSalad = VariantScheme<Variant::FruitSalad>;

class SchemeCmf : public DynamicScheme {
public:
    SchemeCmf(std::vector<Hct> sourceColourHcts, bool isDark, double contrastLevel,
        SpecVersion specVersion = SpecVersion::Spec2026, Platform platform = Platform::Phone);

    SchemeCmf(const Hct& sourceColourHct, bool isDark, double contrastLevel,
        SpecVersion specVersion = SpecVersion::Spec2026, Platform platform = Platform::Phone)
        : SchemeCmf(std::vector{ sourceColourHct }, isDark, contrastLevel, specVersion, platform) {}

    static double getErrorHue(double primaryHue, double tertiaryHue);
};

} // namespace m3colours::scheme
