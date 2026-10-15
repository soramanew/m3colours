#include <gtest/gtest.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "m3colours/contrast.hpp"
#include "m3colours/dynamiccolour.hpp"

namespace m3colours::dynamiccolour {

namespace {

constexpr Argb k_seeds[]{
    Argb{ 0xffff0000 },
    Argb{ 0xffffff00 },
    Argb{ 0xff00ff00 },
    Argb{ 0xff0000ff },
};
constexpr double k_contrastLevels[]{
    -1.0,
    -0.75,
    -0.5,
    -0.25,
    0.0,
    0.25,
    0.5,
    0.75,
    1.0,
};
constexpr Variant k_variants[]{
    Variant::Content,
    Variant::Expressive,
    Variant::Fidelity,
    Variant::Monochrome,
    Variant::Neutral,
    Variant::TonalSpot,
    Variant::Vibrant,
};

const std::vector<const DynamicColour*>& colours() {
    static const std::vector<const DynamicColour*> k_colours{
        &material::background(),
        &material::onBackground(),
        &material::surface(),
        &material::surfaceDim(),
        &material::surfaceBright(),
        &material::surfaceContainerLowest(),
        &material::surfaceContainerLow(),
        &material::surfaceContainer(),
        &material::surfaceContainerHigh(),
        &material::surfaceContainerHighest(),
        &material::onSurface(),
        &material::surfaceVariant(),
        &material::onSurfaceVariant(),
        &material::inverseSurface(),
        &material::inverseOnSurface(),
        &material::outline(),
        &material::outlineVariant(),
        &material::shadow(),
        &material::scrim(),
        &material::surfaceTint(),
        &material::primary(),
        &material::onPrimary(),
        &material::primaryContainer(),
        &material::onPrimaryContainer(),
        &material::inversePrimary(),
        &material::secondary(),
        &material::onSecondary(),
        &material::secondaryContainer(),
        &material::onSecondaryContainer(),
        &material::tertiary(),
        &material::onTertiary(),
        &material::tertiaryContainer(),
        &material::onTertiaryContainer(),
        &material::error(),
        &material::onError(),
        &material::errorContainer(),
        &material::onErrorContainer(),
        &material::primaryFixed(),
        &material::primaryFixedDim(),
        &material::onPrimaryFixed(),
        &material::onPrimaryFixedVariant(),
        &material::secondaryFixed(),
        &material::secondaryFixedDim(),
        &material::onSecondaryFixed(),
        &material::onSecondaryFixedVariant(),
        &material::tertiaryFixed(),
        &material::tertiaryFixedDim(),
        &material::onTertiaryFixed(),
        &material::onTertiaryFixedVariant(),
    };
    return k_colours;
}

const DynamicColour& colourByName(const std::string& name) {
    for (const auto* colour : colours()) {
        if (colour->name() == name)
            return *colour;
    }
    throw std::invalid_argument("Unknown colour " + name);
}

std::vector<DynamicScheme> makeSchemes() {
    std::vector<DynamicScheme> schemes;
    for (const auto seed : k_seeds) {
        for (const auto contrastLevel : k_contrastLevels) {
            for (const auto isDark : { false, true }) {
                for (const auto variant : k_variants) {
                    schemes.emplace_back(DynamicSchemeOptions{
                        .sourceColourHcts = { Hct{ seed } },
                        .variant = variant,
                        .contrastLevel = contrastLevel,
                        .isDark = isDark,
                    });
                }
            }
        }
    }
    return schemes;
}

std::string describe(const DynamicScheme& scheme) {
    return "seed=" + std::to_string(scheme.sourceColourArgb().value()) +
           " variant=" + std::to_string(static_cast<int>(scheme.variant())) +
           " contrast=" + std::to_string(scheme.contrastLevel()) + (scheme.isDark() ? " dark" : " light");
}

double minRequirement(const ContrastCurve& curve, double level) {
    if (level >= 1.0)
        return curve.high;
    if (level >= 0.5)
        return curve.medium;
    if (level >= 0.0)
        return curve.normal;
    return curve.low;
}

using Pairs = std::vector<std::pair<std::string, std::string>>;

Pairs getPairs(bool respectively, const std::vector<std::string>& fores, const std::vector<std::string>& backs) {
    Pairs pairs;
    if (respectively) {
        for (std::size_t i = 0; i < fores.size(); i++)
            pairs.emplace_back(fores[i], backs[i]);
    } else {
        for (const auto& fore : fores) {
            for (const auto& back : backs)
                pairs.emplace_back(fore, back);
        }
    }
    return pairs;
}

struct ContrastCheck {
    ContrastCurve values;
    std::vector<std::string> fore;
    std::vector<std::string> back;
    bool respectively = false;
};

struct DeltaCheck {
    double delta;
    std::vector<std::string> fore;
    std::vector<std::string> back;
    TonePolarity polarity;
};

const std::vector<std::string> k_limitingSurfaces{
    "surface_dim",
    "surface_bright",
};

const std::vector<ContrastCheck> k_contrastChecks{
    { .values = ContrastCurve{ 4.5, 7, 11, 21 }, .fore = { "on_surface" }, .back = k_limitingSurfaces },
    { .values = ContrastCurve{ 3, 4.5, 7, 11 }, .fore = { "on_surface_variant" }, .back = k_limitingSurfaces },
    { .values = ContrastCurve{ 3, 4.5, 7, 7 },
        .fore = { "primary", "secondary", "tertiary", "error" },
        .back = k_limitingSurfaces, },
    { .values = ContrastCurve{ 1.5, 3, 4.5, 7 }, .fore = { "outline" }, .back = k_limitingSurfaces },
    { .values = ContrastCurve{ 0, 0, 3, 4.5 },
        .fore = { "primary_container", "primary_fixed", "primary_fixed_dim", "secondary_container", "secondary_fixed",
            "secondary_fixed_dim", "tertiary_container", "tertiary_fixed", "tertiary_fixed_dim", "error_container",
            "outline_variant", },
        .back = k_limitingSurfaces, },
    { .values = ContrastCurve{ 4.5, 7, 11, 21 }, .fore = { "inverse_on_surface" }, .back = { "inverse_surface" } },
    { .values = ContrastCurve{ 3, 4.5, 7, 7 }, .fore = { "inverse_primary" }, .back = { "inverse_surface" } },
    { .values = ContrastCurve{ 4.5, 7, 11, 21 },
        .fore = { "on_primary", "on_secondary", "on_tertiary", "on_error" },
        .back = { "primary", "secondary", "tertiary", "error" },
        .respectively = true, },
    { .values = ContrastCurve{ 3, 4.5, 7, 11 },
        .fore = { "on_primary_container", "on_secondary_container", "on_tertiary_container", "on_error_container" },
        .back = { "primary_container", "secondary_container", "tertiary_container", "error_container" },
        .respectively = true, },
    { .values = ContrastCurve{ 4.5, 7, 11, 21 },
        .fore = { "on_primary_fixed" },
        .back = { "primary_fixed", "primary_fixed_dim" }, },
    { .values = ContrastCurve{ 4.5, 7, 11, 21 },
        .fore = { "on_secondary_fixed" },
        .back = { "secondary_fixed", "secondary_fixed_dim" }, },
    { .values = ContrastCurve{ 4.5, 7, 11, 21 },
        .fore = { "on_tertiary_fixed" },
        .back = { "tertiary_fixed", "tertiary_fixed_dim" }, },
    { .values = ContrastCurve{ 3, 4.5, 7, 11 },
        .fore = { "on_primary_fixed_variant" },
        .back = { "primary_fixed", "primary_fixed_dim" }, },
    { .values = ContrastCurve{ 3, 4.5, 7, 11 },
        .fore = { "on_secondary_fixed_variant" },
        .back = { "secondary_fixed", "secondary_fixed_dim" }, },
    { .values = ContrastCurve{ 3, 4.5, 7, 11 },
        .fore = { "on_tertiary_fixed_variant" },
        .back = { "tertiary_fixed", "tertiary_fixed_dim" }, },
};

const std::vector<DeltaCheck> k_deltaChecks{
    {
        .delta = 10.0,
        .fore = { "primary", "secondary", "tertiary", "error" },
        .back = { "primary_container", "secondary_container", "tertiary_container", "error_container" },
        .polarity = TonePolarity::Farther,
    },
    {
        .delta = 10.0,
        .fore = { "primary_fixed_dim", "secondary_fixed_dim", "tertiary_fixed_dim" },
        .back = { "primary_fixed", "secondary_fixed", "tertiary_fixed" },
        .polarity = TonePolarity::Darker,
    },
};

const std::vector<std::string> k_backgroundChecks{
    "background",
    "error",
    "error_container",
    "primary",
    "primary_container",
    "primary_fixed",
    "primary_fixed_dim",
    "secondary",
    "secondary_container",
    "secondary_fixed",
    "secondary_fixed_dim",
    "surface",
    "surface_bright",
    "surface_container",
    "surface_container_high",
    "surface_container_highest",
    "surface_container_low",
    "surface_container_lowest",
    "surface_dim",
    "surface_tint",
    "surface_variant",
    "tertiary",
    "tertiary_container",
    "tertiary_fixed",
    "tertiary_fixed_dim",
};

TEST(DynamicColour, GeneratesColoursRespectingContrast) {
    const std::vector<std::pair<std::string, std::string>> textSurfacePairs{
        {
            "on_primary",
            "primary",
        },
        {
            "on_primary_container",
            "primary_container",
        },
        {
            "on_secondary",
            "secondary",
        },
        {
            "on_secondary_container",
            "secondary_container",
        },
        {
            "on_tertiary",
            "tertiary",
        },
        {
            "on_tertiary_container",
            "tertiary_container",
        },
        {
            "on_error",
            "error",
        },
        {
            "on_error_container",
            "error_container",
        },
        {
            "on_background",
            "background",
        },
        {
            "on_surface_variant",
            "surface_bright",
        },
        {
            "on_surface_variant",
            "surface_dim",
        },
    };

    for (const auto& scheme : makeSchemes()) {
        for (const auto& [fgName, bgName] : textSurfacePairs) {
            const auto foregroundTone = colourByName(fgName).getHct(scheme).tone();
            const auto backgroundTone = colourByName(bgName).getHct(scheme).tone();
            const auto contrast = contrast::ratioOfTones(foregroundTone, backgroundTone);
            const auto minimumRequirement = scheme.contrastLevel() >= 0.0 ? 4.5 : 3.0;
            EXPECT_GE(contrast, minimumRequirement) << fgName << " on " << bgName << " for " << describe(scheme);
        }
    }
}

std::map<std::string, double> resolveTones(const DynamicScheme& scheme) {
    std::map<std::string, double> tones;
    for (const auto* colour : colours())
        tones[colour->name()] = lstarFromArgb(colour->getArgb(scheme));
    return tones;
}

TEST(DynamicColour, ContrastConstraints) {
    constexpr auto k_contrastTolerance = 0.05;

    for (const auto& scheme : makeSchemes()) {
        const auto tones = resolveTones(scheme);
        for (const auto& check : k_contrastChecks) {
            const auto minRequired = minRequirement(check.values, scheme.contrastLevel());
            for (const auto& [fore, back] : getPairs(check.respectively, check.fore, check.back)) {
                const auto ftone = tones.at(fore);
                const auto btone = tones.at(back);
                const auto contrast = contrast::ratioOfTones(ftone, btone);
                const auto belowRequirement = contrast < minRequired - k_contrastTolerance;
                const auto saturated = ftone <= 0.0 || btone <= 0.0 || ftone >= 100.0 || btone >= 100.0;
                const auto failing = minRequired <= 4.5 ? belowRequirement : (belowRequirement && !saturated);
                EXPECT_FALSE(failing) << "Contrast " << fore << " " << ftone << " " << back << " " << btone << " "
                                      << contrast << " " << minRequired << " for " << describe(scheme);
            }
        }
    }
}

TEST(DynamicColour, DeltaConstraints) {
    for (const auto& scheme : makeSchemes()) {
        const auto tones = resolveTones(scheme);
        for (const auto& check : k_deltaChecks) {
            for (const auto& [fore, back] : getPairs(true, check.fore, check.back)) {
                const auto ftone = tones.at(fore);
                const auto btone = tones.at(back);
                const auto isLighter = check.polarity == TonePolarity::Lighter ||
                                       (check.polarity == TonePolarity::Nearer && !scheme.isDark()) ||
                                       (check.polarity == TonePolarity::Farther && scheme.isDark());
                const auto observedDelta = isLighter ? ftone - btone : btone - ftone;
                EXPECT_GE(observedDelta, check.delta - 0.5)
                    << "Delta " << fore << " " << ftone << " " << back << " " << btone << " for " << describe(scheme);
            }
        }
    }
}

TEST(DynamicColour, BackgroundsAvoidAwkwardZone) {
    for (const auto& scheme : makeSchemes()) {
        const auto tones = resolveTones(scheme);
        for (const auto& name : k_backgroundChecks) {
            const auto tone = tones.at(name);
            EXPECT_FALSE(tone >= 50.5 && tone < 59.5)
                << "Background " << name << " " << tone << " for " << describe(scheme);
        }
    }
}

} // namespace

} // namespace m3colours::dynamiccolour
