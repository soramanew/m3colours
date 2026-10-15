#include <algorithm>
#include <stdexcept>

#include "m3colours/scheme.hpp"

namespace m3colours::scheme {

namespace {

double hueByTertiaryBand(double tertiaryHue, double low, double high, double below, double within, double above) {
    if (tertiaryHue <= low)
        return below;
    if (tertiaryHue <= high)
        return within;
    return above;
}

using palettes::TonalPalette;

DynamicSchemeOptions cmfOptions(
    std::vector<Hct> sourceColourHcts, bool isDark, double contrastLevel, SpecVersion specVersion, Platform platform) {
    if (specVersion != SpecVersion::Spec2026)
        throw std::invalid_argument("SchemeCmf can only be used with spec version 2026");
    if (sourceColourHcts.empty())
        throw std::invalid_argument("sourceColourHcts cannot be empty");

    const auto& source = sourceColourHcts.front();
    const auto& secondarySource = sourceColourHcts.size() > 1 ? sourceColourHcts[1] : source;
    const auto hue = source.hue();
    const auto chroma = source.chroma();
    const auto sameSource = source.toArgb() == secondarySource.toArgb();

    return {
        .sourceColourHcts = std::move(sourceColourHcts),
        .variant = Variant::Cmf,
        .contrastLevel = contrastLevel,
        .isDark = isDark,
        .platform = platform,
        .specVersion = specVersion,
        .primaryPalette = TonalPalette::fromHueAndChroma(hue, chroma),
        .secondaryPalette = TonalPalette::fromHueAndChroma(hue, chroma * 0.5),
        .tertiaryPalette = sameSource ? TonalPalette::fromHueAndChroma(hue, chroma * 0.75)
                                      : TonalPalette::fromHueAndChroma(secondarySource.hue(), secondarySource.chroma()),
        .neutralPalette = TonalPalette::fromHueAndChroma(hue, chroma * 0.2),
        .neutralVariantPalette = TonalPalette::fromHueAndChroma(hue, chroma * 0.2),
        .errorPalette =
            TonalPalette::fromHueAndChroma(SchemeCmf::getErrorHue(hue, secondarySource.hue()), std::max(chroma, 50.0)),
    };
}

} // namespace

SchemeCmf::SchemeCmf(
    std::vector<Hct> sourceColourHcts, bool isDark, double contrastLevel, SpecVersion specVersion, Platform platform)
    : DynamicScheme(cmfOptions(std::move(sourceColourHcts), isDark, contrastLevel, specVersion, platform)) {}

double SchemeCmf::getErrorHue(double primaryHue, double tertiaryHue) {
    if (primaryHue <= 8.0)
        return hueByTertiaryBand(tertiaryHue, 24.0, 32.0, 28.0, 16.0, 20.0);
    if (primaryHue <= 16.0)
        return hueByTertiaryBand(tertiaryHue, 24.0, 32.0, 32.0, 20.0, 24.0);
    if (primaryHue <= 20.0)
        return hueByTertiaryBand(tertiaryHue, 28.0, 32.0, 32.0, 24.0, 28.0);
    if (primaryHue <= 28.0)
        return tertiaryHue <= 24.0 ? 32.0 : 16.0;
    if (primaryHue <= 32.0)
        return hueByTertiaryBand(tertiaryHue, 20.0, 28.0, 24.0, 16.0, 20.0);
    if (primaryHue <= 40.0)
        return hueByTertiaryBand(tertiaryHue, 20.0, 28.0, 24.0, 16.0, 24.0);
    if (primaryHue <= 152.0)
        return hueByTertiaryBand(tertiaryHue, 24.0, 36.0, 32.0, 20.0, 32.0);
    if (primaryHue <= 272.0)
        return hueByTertiaryBand(tertiaryHue, 20.0, 28.0, 24.0, 16.0, 24.0);
    return hueByTertiaryBand(tertiaryHue, 12.0, 28.0, 16.0, 32.0, 16.0);
}

} // namespace m3colours::scheme
