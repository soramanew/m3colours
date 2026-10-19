#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include "m3colours/dislike.hpp"
#include "m3colours/dynamiccolour.hpp"

// Spec 2026 is only reachable with Variant::Cmf (see DynamicScheme), so the 2026 layers assume it.
namespace m3colours::dynamiccolour::material {

namespace {

using palettes::TonalPalette;

// clang-format off
const TonalPalette& primaryPalette(const DynamicScheme& s) { return s.primaryPalette(); }
const TonalPalette& secondaryPalette(const DynamicScheme& s) { return s.secondaryPalette(); }
const TonalPalette& tertiaryPalette(const DynamicScheme& s) { return s.tertiaryPalette(); }
const TonalPalette& neutralPalette(const DynamicScheme& s) { return s.neutralPalette(); }
const TonalPalette& neutralVariantPalette(const DynamicScheme& s) { return s.neutralVariantPalette(); }
const TonalPalette& errorPalette(const DynamicScheme& s) { return s.errorPalette(); }

bool isPhone(const DynamicScheme& s) { return s.platform() == Platform::Phone; }
bool isMonochrome(const DynamicScheme& s) { return s.variant() == Variant::Monochrome; }
bool isFidelity(const DynamicScheme& s) { return s.variant() == Variant::Fidelity || s.variant() == Variant::Content; }
bool isVibrant(const DynamicScheme& s) { return s.variant() == Variant::Vibrant; }
bool isYellowNeutral(const DynamicScheme& s) { return Hct::isYellow(s.neutralPalette().hue()); }
// clang-format on

constexpr ContrastCurve k_curve1p5{ 1.5, 1.5, 3.0, 5.5 };
constexpr ContrastCurve k_curve3{ 3.0, 3.0, 4.5, 7.0 };
constexpr ContrastCurve k_curve4p5{ 4.5, 4.5, 7.0, 11.0 };
constexpr ContrastCurve k_curve6{ 6.0, 6.0, 7.0, 11.0 };
constexpr ContrastCurve k_curve7{ 7.0, 7.0, 11.0, 21.0 };
constexpr ContrastCurve k_curve9{ 9.0, 9.0, 11.0, 21.0 };
constexpr ContrastCurve k_curve11{ 11.0, 11.0, 21.0, 21.0 };

std::optional<ContrastCurve> containerCurve2026(const DynamicScheme& s) {
    if (s.contrastLevel() > 0.0)
        return k_curve1p5;
    return std::nullopt;
}

std::optional<ContrastCurve> containerCurve2025(const DynamicScheme& s) {
    if (isPhone(s))
        return containerCurve2026(s);
    return std::nullopt;
}

const DynamicColour* phoneHighestSurface(const DynamicScheme& s) {
    return isPhone(s) ? &highestSurface(s) : nullptr;
}

const DynamicColour* highestSurfaceOrContainerHigh(const DynamicScheme& s) {
    return isPhone(s) ? &highestSurface(s) : &surfaceContainerHigh();
}

double chromaAt(double hue, double chroma, double tone) {
    return hct::internal::jchFromXyz(
        hct::internal::xyzFromArgb(hct::solveToArgb(hue, chroma, tone)), hct::k_defaultViewingConditions)
        .chroma;
}

double tMaxC(const TonalPalette& palette, double lowerBound = 0.0, double upperBound = 100.0) {
    return std::clamp(palette.toneOfMaxChroma(), lowerBound, upperBound);
}

double tMaxC(const TonalPalette& palette, double lowerBound, double upperBound, double chromaMultiplier) {
    return tMaxC(
        TonalPalette::fromHueAndChroma(palette.hue(), palette.chroma() * chromaMultiplier), lowerBound, upperBound);
}

double tMinC(const TonalPalette& palette, double lowerBound = 0.0, double upperBound = 100.0) {
    return std::clamp(palette.toneOfMinChroma(), lowerBound, upperBound);
}

double findDesiredChromaByTone(double hue, double chroma, double tone, bool byDecreasingTone) {
    auto answer = tone;
    auto closestChroma = chromaAt(hue, chroma, tone);
    if (closestChroma >= chroma)
        return answer;

    auto chromaPeak = closestChroma;
    while (closestChroma < chroma) {
        answer += byDecreasingTone ? -1.0 : 1.0;
        const auto potentialChroma = chromaAt(hue, chroma, answer);
        if (chromaPeak > potentialChroma)
            break;
        if (std::abs(potentialChroma - chroma) < 0.4)
            break;
        if (std::abs(potentialChroma - chroma) < std::abs(closestChroma - chroma))
            closestChroma = potentialChroma;
        chromaPeak = std::max(chromaPeak, potentialChroma);
    }
    return answer;
}

double neutralForegroundChromaMultiplier2025(const DynamicScheme& s) {
    if (!isPhone(s))
        return 1.0;
    switch (s.variant()) {
    case Variant::Neutral:
        return 2.2;
    case Variant::TonalSpot:
        return 1.7;
    case Variant::Expressive:
        if (!isYellowNeutral(s))
            return 1.6;
        return s.isDark() ? 3.0 : 2.3;
    default:
        return 1.0;
    }
}

double surfaceChromaMultiplier2025(const DynamicScheme& s, double neutral, double tonalSpot, double expressiveYellow,
    double expressive, double vibrant) {
    switch (s.variant()) {
    case Variant::Neutral:
        return neutral;
    case Variant::TonalSpot:
        return tonalSpot;
    case Variant::Expressive:
        return isYellowNeutral(s) ? expressiveYellow : expressive;
    case Variant::Vibrant:
        return vibrant;
    default:
        return 1.0;
    }
}

double lightSurfaceTone2025(const DynamicScheme& s, double yellow, double vibrant, double other) {
    if (isYellowNeutral(s))
        return yellow;
    return isVibrant(s) ? vibrant : other;
}

const Hct& secondarySourceColourHct(const DynamicScheme& s) {
    const auto& hcts = s.sourceColourHcts();
    return hcts.size() > 1 ? hcts[1] : hcts[0];
}

const DynamicColour* fixedBackground2026(double fixedTone, const DynamicColour& fixed, const DynamicColour& fixedDim) {
    return fixedTone > 57.0 ? &fixedDim : &fixed;
}

enum Role : std::uint8_t {
    PrimaryPaletteKeyColour,
    SecondaryPaletteKeyColour,
    TertiaryPaletteKeyColour,
    NeutralPaletteKeyColour,
    NeutralVariantPaletteKeyColour,
    ErrorPaletteKeyColour,
    Background,
    OnBackground,
    Surface,
    SurfaceDim,
    SurfaceBright,
    SurfaceContainerLowest,
    SurfaceContainerLow,
    SurfaceContainer,
    SurfaceContainerHigh,
    SurfaceContainerHighest,
    OnSurface,
    SurfaceVariant,
    OnSurfaceVariant,
    InverseSurface,
    InverseOnSurface,
    Outline,
    OutlineVariant,
    Shadow,
    Scrim,
    SurfaceTint,
    Primary,
    PrimaryDim,
    OnPrimary,
    PrimaryContainer,
    OnPrimaryContainer,
    InversePrimary,
    PrimaryFixed,
    PrimaryFixedDim,
    OnPrimaryFixed,
    OnPrimaryFixedVariant,
    Secondary,
    SecondaryDim,
    OnSecondary,
    SecondaryContainer,
    OnSecondaryContainer,
    SecondaryFixed,
    SecondaryFixedDim,
    OnSecondaryFixed,
    OnSecondaryFixedVariant,
    Tertiary,
    TertiaryDim,
    OnTertiary,
    TertiaryContainer,
    OnTertiaryContainer,
    TertiaryFixed,
    TertiaryFixedDim,
    OnTertiaryFixed,
    OnTertiaryFixedVariant,
    Error,
    ErrorDim,
    OnError,
    ErrorContainer,
    OnErrorContainer,
    Count,
};

static_assert(Role::Count <= k_toneCacheSize);

} // namespace

const DynamicColour& highestSurface(const DynamicScheme& scheme) {
    return scheme.isDark() ? surfaceBright() : surfaceDim();
}

////////////////////////////////////////////////////////////////
// Main Palettes                                              //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_primaryPaletteKeyColour{ Role::PrimaryPaletteKeyColour, {
    .name = "primary_palette_key_color",
    .base = {
        .palette = primaryPalette,
        .tone = [](const DynamicScheme& s) { return s.primaryPalette().keyColour().tone(); },
    },
} };

constexpr DynamicColour k_secondaryPaletteKeyColour{ Role::SecondaryPaletteKeyColour, {
    .name = "secondary_palette_key_color",
    .base = {
        .palette = secondaryPalette,
        .tone = [](const DynamicScheme& s) { return s.secondaryPalette().keyColour().tone(); },
    },
} };

constexpr DynamicColour k_tertiaryPaletteKeyColour{ Role::TertiaryPaletteKeyColour, {
    .name = "tertiary_palette_key_color",
    .base = {
        .palette = tertiaryPalette,
        .tone = [](const DynamicScheme& s) { return s.tertiaryPalette().keyColour().tone(); },
    },
} };

constexpr DynamicColour k_neutralPaletteKeyColour{ Role::NeutralPaletteKeyColour, {
    .name = "neutral_palette_key_color",
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.neutralPalette().keyColour().tone(); },
    },
} };

constexpr DynamicColour k_neutralVariantPaletteKeyColour{ Role::NeutralVariantPaletteKeyColour, {
    .name = "neutral_variant_palette_key_color",
    .base = {
        .palette = neutralVariantPalette,
        .tone = [](const DynamicScheme& s) { return s.neutralVariantPalette().keyColour().tone(); },
    },
} };

constexpr DynamicColour k_errorPaletteKeyColour{ Role::ErrorPaletteKeyColour, {
    .name = "error_palette_key_color",
    .base = {
        .palette = errorPalette,
        .tone = [](const DynamicScheme& s) { return s.errorPalette().keyColour().tone(); },
    },
} };

////////////////////////////////////////////////////////////////
// Surfaces [S]                                               //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_background{ Role::Background, {
    .name = "background",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 6.0 : 98.0; },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return surface().getTone(s); },
    },
} };

constexpr DynamicColour k_onBackground{ Role::OnBackground,
    {
        .name = "on_background",
        .base = {
            .palette = neutralPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 90.0 : 10.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &background();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 3.0, 4.5, 7.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = neutralPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isPhone(s) ? onSurface().getTone(s) : 100.0;
                },
            .chromaMultiplier =
                [](const DynamicScheme& s) {
                    return onSurface().chromaMultiplier(s);
                },
            .background =
                [](const DynamicScheme& s) {
                    return onSurface().background(s);
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return onSurface().contrastCurve(s);
            },
        },
    } };

constexpr DynamicColour k_surface{ Role::Surface, {
    .name = "surface",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 6.0 : 98.0; },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            if (!isPhone(s))
                return 0.0;
            return s.isDark() ? 4.0 : lightSurfaceTone2025(s, 99.0, 97.0, 98.0);
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 4.0 : 98.0; },
    },
} };

constexpr DynamicColour k_surfaceDim{ Role::SurfaceDim, {
    .name = "surface_dim",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? 6.0 : ContrastCurve{ 87.0, 87.0, 80.0, 75.0 }.get(s.contrastLevel());
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 4.0 : lightSurfaceTone2025(s, 90.0, 85.0, 87.0); },
        .chromaMultiplier = [](const DynamicScheme& s) {
            return s.isDark() ? 1.0 : surfaceChromaMultiplier2025(s, 2.5, 1.7, 2.7, 1.75, 1.36);
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 4.0 : 87.0; },
        .chromaMultiplier = [](const DynamicScheme& s) { return s.isDark() ? 1.0 : 1.7; },
    },
} };

constexpr DynamicColour k_surfaceBright{ Role::SurfaceBright, {
    .name = "surface_bright",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? ContrastCurve{ 24.0, 24.0, 29.0, 34.0 }.get(s.contrastLevel()) : 98.0;
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 18.0 : lightSurfaceTone2025(s, 99.0, 97.0, 98.0); },
        .chromaMultiplier = [](const DynamicScheme& s) {
            return s.isDark() ? surfaceChromaMultiplier2025(s, 2.5, 1.7, 2.7, 1.75, 1.36) : 1.0;
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 18.0 : 98.0; },
        .chromaMultiplier = [](const DynamicScheme& s) { return s.isDark() ? 1.7 : 1.0; },
    },
} };

constexpr DynamicColour k_surfaceContainerLowest{ Role::SurfaceContainerLowest, {
    .name = "surface_container_lowest",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? ContrastCurve{ 4.0, 4.0, 2.0, 0.0 }.get(s.contrastLevel()) : 100.0;
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 0.0 : 100.0; },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 0.0 : 100.0; },
    },
} };

constexpr DynamicColour k_surfaceContainerLow{ Role::SurfaceContainerLow, {
    .name = "surface_container_low",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? ContrastCurve{ 10.0, 10.0, 11.0, 12.0 }.get(s.contrastLevel())
                              : ContrastCurve{ 96.0, 96.0, 96.0, 95.0 }.get(s.contrastLevel());
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            if (!isPhone(s))
                return 15.0;
            return s.isDark() ? 6.0 : lightSurfaceTone2025(s, 98.0, 95.0, 96.0);
        },
        .chromaMultiplier = [](const DynamicScheme& s) {
            return isPhone(s) ? surfaceChromaMultiplier2025(s, 1.3, 1.25, 1.3, 1.15, 1.08) : 1.0;
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 6.0 : 96.0; },
        .chromaMultiplier = [](const DynamicScheme&) { return 1.25; },
    },
} };

constexpr DynamicColour k_surfaceContainer{ Role::SurfaceContainer, {
    .name = "surface_container",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? ContrastCurve{ 12.0, 12.0, 16.0, 20.0 }.get(s.contrastLevel())
                              : ContrastCurve{ 94.0, 94.0, 92.0, 90.0 }.get(s.contrastLevel());
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            if (!isPhone(s))
                return 20.0;
            return s.isDark() ? 9.0 : lightSurfaceTone2025(s, 96.0, 92.0, 94.0);
        },
        .chromaMultiplier = [](const DynamicScheme& s) {
            return isPhone(s) ? surfaceChromaMultiplier2025(s, 1.6, 1.4, 1.6, 1.3, 1.15) : 1.0;
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 9.0 : 94.0; },
        .chromaMultiplier = [](const DynamicScheme&) { return 1.4; },
    },
} };

constexpr DynamicColour k_surfaceContainerHigh{ Role::SurfaceContainerHigh, {
    .name = "surface_container_high",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? ContrastCurve{ 17.0, 17.0, 21.0, 25.0 }.get(s.contrastLevel())
                              : ContrastCurve{ 92.0, 92.0, 88.0, 85.0 }.get(s.contrastLevel());
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            if (!isPhone(s))
                return 25.0;
            return s.isDark() ? 12.0 : lightSurfaceTone2025(s, 94.0, 90.0, 92.0);
        },
        .chromaMultiplier = [](const DynamicScheme& s) {
            return isPhone(s) ? surfaceChromaMultiplier2025(s, 1.9, 1.5, 1.95, 1.45, 1.22) : 1.0;
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 12.0 : 92.0; },
        .chromaMultiplier = [](const DynamicScheme&) { return 1.5; },
    },
} };

constexpr DynamicColour k_surfaceContainerHighest{ Role::SurfaceContainerHighest, {
    .name = "surface_container_highest",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) {
            return s.isDark() ? ContrastCurve{ 22.0, 22.0, 26.0, 30.0 }.get(s.contrastLevel())
                              : ContrastCurve{ 90.0, 90.0, 84.0, 80.0 }.get(s.contrastLevel());
        },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 15.0 : lightSurfaceTone2025(s, 92.0, 88.0, 90.0); },
        .chromaMultiplier = [](const DynamicScheme& s) {
            return surfaceChromaMultiplier2025(s, 2.2, 1.7, 2.3, 1.6, 1.29);
        },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 15.0 : 90.0; },
        .chromaMultiplier = [](const DynamicScheme&) { return 1.7; },
    },
} };

constexpr DynamicColour k_onSurface{ Role::OnSurface,
    {
        .name = "on_surface",
        .base = {
            .palette = neutralPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 90.0 : 10.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = neutralPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isVibrant(s))
                        return tMaxC(s.neutralPalette(), 0.0, 100.0, 1.1);
                    return highestSurfaceOrContainerHigh(s)->getTone(s);
                },
            .chromaMultiplier = neutralForegroundChromaMultiplier2025,
            .background = highestSurfaceOrContainerHigh,
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return s.isDark() && isPhone(s) ? k_curve11 : k_curve9;
            },
        },
        .spec2026 = ColourSpec{
            .palette = neutralPalette,
            .chromaMultiplier =
                [](const DynamicScheme&) {
                    return 1.7;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return s.isDark() ? k_curve11 : k_curve9;
            },
        },
    } };

constexpr DynamicColour k_surfaceVariant{ Role::SurfaceVariant, {
    .name = "surface_variant",
    .isBackground = true,
    .base = {
        .palette = neutralVariantPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 30.0 : 90.0; },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return surfaceContainerHighest().getTone(s); },
        .chromaMultiplier = [](const DynamicScheme& s) { return surfaceContainerHighest().chromaMultiplier(s); },
    },
} };

constexpr DynamicColour k_onSurfaceVariant{ Role::OnSurfaceVariant,
    {
        .name = "on_surface_variant",
        .base = {
            .palette = neutralVariantPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 80.0 : 30.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 =
            ColourSpec{
                .palette = neutralPalette,
                .chromaMultiplier = neutralForegroundChromaMultiplier2025,
                .background = highestSurfaceOrContainerHigh,
                .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                    if (!isPhone(s))
                        return k_curve7;
                    return s.isDark() ? k_curve6 : k_curve4p5;
                },
            },
        .spec2026 = ColourSpec{
            .palette = neutralPalette,
            .chromaMultiplier =
                [](const DynamicScheme&) {
                    return 1.7;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return s.isDark() ? k_curve6 : k_curve4p5;
            },
        },
    } };

constexpr DynamicColour k_inverseSurface{ Role::InverseSurface, {
    .name = "inverse_surface",
    .isBackground = true,
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 90.0 : 20.0; },
    },
    .spec2025 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 98.0 : 4.0; },
    },
    .spec2026 = ColourSpec{
        .palette = neutralPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 98.0 : 4.0; },
        .chromaMultiplier = [](const DynamicScheme&) { return 1.7; },
    },
} };

constexpr DynamicColour k_inverseOnSurface{ Role::InverseOnSurface,
    {
        .name = "inverse_on_surface",
        .base = {
            .palette = neutralPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 20.0 : 95.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &inverseSurface();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = neutralPalette,
            .background =
                [](const DynamicScheme&) {
                    return &inverseSurface();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
    } };

constexpr DynamicColour k_outline{ Role::Outline,
    {
        .name = "outline",
        .base = {
            .palette = neutralVariantPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 60.0 : 50.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.5, 3.0, 4.5, 7.0 };
            },
        },
        .spec2025 =
            ColourSpec{
                .palette = neutralPalette,
                .chromaMultiplier = neutralForegroundChromaMultiplier2025,
                .background = highestSurfaceOrContainerHigh,
                .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                    return isPhone(s) ? k_curve3 : k_curve4p5;
                },
            },
        .spec2026 = ColourSpec{
            .palette = neutralPalette,
            .chromaMultiplier =
                [](const DynamicScheme&) {
                    return 1.7;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve3;
            },
        },
    } };

constexpr DynamicColour k_outlineVariant{ Role::OutlineVariant,
    {
        .name = "outline_variant",
        .base = {
            .palette = neutralVariantPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 30.0 : 80.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
        },
        .spec2025 =
            ColourSpec{
                .palette = neutralPalette,
                .chromaMultiplier = neutralForegroundChromaMultiplier2025,
                .background = highestSurfaceOrContainerHigh,
                .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                    return isPhone(s) ? k_curve1p5 : k_curve3;
                },
            },
        .spec2026 = ColourSpec{
            .palette = neutralPalette,
            .chromaMultiplier =
                [](const DynamicScheme&) {
                    return 1.7;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve1p5;
            },
        },
    } };

constexpr DynamicColour k_shadow{ Role::Shadow, {
    .name = "shadow",
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme&) { return 0.0; },
    },
} };

constexpr DynamicColour k_scrim{ Role::Scrim, {
    .name = "scrim",
    .base = {
        .palette = neutralPalette,
        .tone = [](const DynamicScheme&) { return 0.0; },
    },
} };

constexpr DynamicColour k_surfaceTint{ Role::SurfaceTint, {
    .name = "surface_tint",
    .isBackground = true,
    .base = {
        .palette = primaryPalette,
        .tone = [](const DynamicScheme& s) { return s.isDark() ? 80.0 : 40.0; },
    },
    .spec2025 = ColourSpec{
        .palette = primaryPalette,
        .tone = [](const DynamicScheme& s) { return primary().getTone(s); },
    },
} };

////////////////////////////////////////////////////////////////
// Primaries [P]                                              //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_primary{ Role::Primary,
    {
        .name = "primary",
        .isBackground = true,
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 100.0 : 0.0;
                    return s.isDark() ? 80.0 : 40.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 7.0 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryContainer(), primary(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.primaryPalette();
                    switch (s.variant()) {
                    case Variant::Neutral:
                        if (!isPhone(s))
                            return 90.0;
                        return s.isDark() ? 80.0 : 40.0;
                    case Variant::TonalSpot:
                        if (!isPhone(s))
                            return tMaxC(palette, 0.0, 90.0);
                        return s.isDark() ? 80.0 : tMaxC(palette);
                    case Variant::Expressive: {
                        if (!isPhone(s))
                            return tMaxC(palette);
                        const auto darkBound = Hct::isCyan(palette.hue()) ? 88.0 : 98.0;
                        const auto lightBound = Hct::isYellow(palette.hue()) ? 25.0 : 98.0;
                        return tMaxC(palette, 0.0, s.isDark() ? darkBound : lightBound);
                    }
                    default:
                        if (!isPhone(s))
                            return tMaxC(palette);
                        return tMaxC(palette, 0.0, Hct::isCyan(palette.hue()) ? 88.0 : 98.0);
                    }
                },
            .background = highestSurfaceOrContainerHigh,
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve4p5 : k_curve7;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ primaryContainer(), primary(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (s.sourceColourHct().chroma() <= 12.0)
                        return s.isDark() ? 80.0 : 40.0;
                    return s.sourceColourHct().tone();
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ primaryContainer(), primary(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_primaryDim{ Role::PrimaryDim,
    {
        .name = "primary_dim",
        .isBackground = true,
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    switch (s.variant()) {
                    case Variant::Neutral:
                        return 85.0;
                    case Variant::TonalSpot:
                        return tMaxC(s.primaryPalette(), 0.0, 90.0);
                    default:
                        return tMaxC(s.primaryPalette());
                    }
                },
            .background =
                [](const DynamicScheme&) {
                    return &surfaceContainerHigh();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryDim(), primary(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 =
            ColourSpec{
                .palette = primaryPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return primary().getTone(s);
                    },
            },
    } };

constexpr DynamicColour k_onPrimary{ Role::OnPrimary,
    {
        .name = "on_primary",
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 10.0 : 90.0;
                    return s.isDark() ? 20.0 : 100.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &primary();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return isPhone(s) ? &primary() : &primaryDim();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &primary();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

constexpr DynamicColour k_primaryContainer{ Role::PrimaryContainer,
    {
        .name = "primary_container",
        .isBackground = true,
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isFidelity(s))
                        return s.sourceColourHct().tone();
                    if (isMonochrome(s))
                        return s.isDark() ? 85.0 : 25.0;
                    return s.isDark() ? 30.0 : 90.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryContainer(), primary(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.primaryPalette();
                    if (!isPhone(s))
                        return 30.0;
                    switch (s.variant()) {
                    case Variant::Neutral:
                        return s.isDark() ? 30.0 : 90.0;
                    case Variant::TonalSpot:
                        return s.isDark() ? tMinC(palette, 35.0, 93.0) : tMaxC(palette, 0.0, 90.0);
                    case Variant::Expressive:
                        return s.isDark() ? tMinC(palette, 30.0, 93.0)
                                          : tMaxC(palette, 78.0, Hct::isCyan(palette.hue()) ? 88.0 : 90.0);
                    default:
                        return s.isDark() ? tMinC(palette, 66.0, 93.0)
                                          : tMaxC(palette, 66.0, Hct::isCyan(palette.hue()) ? 88.0 : 93.0);
                    }
                },
            .background = phoneHighestSurface,
            .contrastCurve = containerCurve2025,
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ primaryContainer(), primaryDim(), 10.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& source = s.sourceColourHct();
                    if (!s.isDark() && source.chroma() <= 12.0)
                        return 90.0;
                    return source.tone() > 55.0 ? std::clamp(source.tone(), 61.0, 90.0)
                                                : std::clamp(source.tone(), 30.0, 49.0);
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

constexpr DynamicColour k_onPrimaryContainer{ Role::OnPrimaryContainer,
    {
        .name = "on_primary_container",
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isFidelity(s))
                        return DynamicColour::foregroundTone(primaryContainer().tone(s), 4.5);
                    if (isMonochrome(s))
                        return s.isDark() ? 0.0 : 100.0;
                    return s.isDark() ? 90.0 : 30.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &primaryContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &primaryContainer();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &primaryContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

constexpr DynamicColour k_inversePrimary{ Role::InversePrimary,
    {
        .name = "inverse_primary",
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 40.0 : 80.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &inverseSurface();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 7.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return tMaxC(s.primaryPalette());
                },
            .background =
                [](const DynamicScheme&) {
                    return &inverseSurface();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Primary Fixed [PF]                                         //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_primaryFixed{ Role::PrimaryFixed,
    {
        .name = "primary_fixed",
        .isBackground = true,
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 40.0 : 90.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryFixed(), primaryFixedDim(), 10.0, TonePolarity::Lighter, true };
            },
        },
        .spec2025 =
            ColourSpec{
                .palette = primaryPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return primaryContainer().getTone(s.asLightStandardContrast());
                    },
                .background = phoneHighestSurface,
                .contrastCurve = containerCurve2025,
            },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return primaryContainer().getTone(s.asLightStandardContrast());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_primaryFixedDim{ Role::PrimaryFixedDim,
    {
        .name = "primary_fixed_dim",
        .isBackground = true,
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 30.0 : 80.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryFixed(), primaryFixedDim(), 10.0, TonePolarity::Lighter, true };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return primaryFixed().getTone(s);
                },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryFixedDim(), primaryFixed(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Exact };
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return primaryFixed().getTone(s);
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ primaryFixedDim(), primaryFixed(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Exact };
            },
        },
    } };

constexpr DynamicColour k_onPrimaryFixed{ Role::OnPrimaryFixed,
    {
        .name = "on_primary_fixed",
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 100.0 : 10.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &primaryFixedDim();
                },
            .secondBackground =
                [](const DynamicScheme&) {
                    return &primaryFixed();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &primaryFixedDim();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return fixedBackground2026(primaryFixed().getTone(s), primaryFixed(), primaryFixedDim());
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
    } };

constexpr DynamicColour k_onPrimaryFixedVariant{ Role::OnPrimaryFixedVariant,
    {
        .name = "on_primary_fixed_variant",
        .base = {
            .palette = primaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 90.0 : 30.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &primaryFixedDim();
                },
            .secondBackground =
                [](const DynamicScheme&) {
                    return &primaryFixed();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &primaryFixedDim();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
        },
        .spec2026 = ColourSpec{
            .palette = primaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return fixedBackground2026(primaryFixed().getTone(s), primaryFixed(), primaryFixedDim());
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Secondaries [Q]                                            //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_secondary{ Role::Secondary,
    {
        .name = "secondary",
        .isBackground = true,
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 80.0 : 40.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 7.0 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryContainer(), secondary(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.secondaryPalette();
                    if (!isPhone(s))
                        return s.variant() == Variant::Neutral ? 90.0 : tMaxC(palette, 0.0, 90.0);
                    switch (s.variant()) {
                    case Variant::Neutral:
                        return s.isDark() ? tMinC(palette, 0.0, 98.0) : tMaxC(palette);
                    case Variant::Vibrant:
                        return tMaxC(palette, 0.0, s.isDark() ? 90.0 : 98.0);
                    default:
                        return s.isDark() ? 80.0 : tMaxC(palette);
                    }
                },
            .background = highestSurfaceOrContainerHigh,
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve4p5 : k_curve7;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ secondaryContainer(), secondary(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? tMinC(s.secondaryPalette()) : tMaxC(s.secondaryPalette());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ secondaryContainer(), secondary(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_secondaryDim{ Role::SecondaryDim,
    {
        .name = "secondary_dim",
        .isBackground = true,
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.variant() == Variant::Neutral ? 85.0 : tMaxC(s.secondaryPalette(), 0.0, 90.0);
                },
            .background =
                [](const DynamicScheme&) {
                    return &surfaceContainerHigh();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryDim(), secondary(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 =
            ColourSpec{
                .palette = secondaryPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return secondary().getTone(s);
                    },
            },
    } };

constexpr DynamicColour k_onSecondary{ Role::OnSecondary,
    {
        .name = "on_secondary",
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 10.0 : 100.0;
                    return s.isDark() ? 20.0 : 100.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &secondary();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return isPhone(s) ? &secondary() : &secondaryDim();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &secondary();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

constexpr DynamicColour k_secondaryContainer{ Role::SecondaryContainer,
    {
        .name = "secondary_container",
        .isBackground = true,
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto initialTone = s.isDark() ? 30.0 : 90.0;
                    if (isMonochrome(s))
                        return s.isDark() ? 30.0 : 85.0;
                    if (!isFidelity(s))
                        return initialTone;
                    const auto& palette = s.secondaryPalette();
                    return findDesiredChromaByTone(palette.hue(), palette.chroma(), initialTone, !s.isDark());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryContainer(), secondary(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.secondaryPalette();
                    if (!isPhone(s))
                        return 30.0;
                    switch (s.variant()) {
                    case Variant::Vibrant:
                        return s.isDark() ? tMinC(palette, 30.0, 40.0) : tMaxC(palette, 84.0, 90.0);
                    case Variant::Expressive:
                        return s.isDark() ? 15.0 : tMaxC(palette, 90.0, 95.0);
                    default:
                        return s.isDark() ? 25.0 : 90.0;
                    }
                },
            .background = phoneHighestSurface,
            .contrastCurve = containerCurve2025,
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ secondaryContainer(), secondaryDim(), 10.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.secondaryPalette();
                    return s.isDark() ? tMinC(palette, 20.0, 49.0) : tMaxC(palette, 61.0, 90.0);
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

constexpr DynamicColour k_onSecondaryContainer{ Role::OnSecondaryContainer,
    {
        .name = "on_secondary_container",
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 90.0 : 10.0;
                    if (!isFidelity(s))
                        return s.isDark() ? 90.0 : 30.0;
                    return DynamicColour::foregroundTone(secondaryContainer().tone(s), 4.5);
                },
            .background =
                [](const DynamicScheme&) {
                    return &secondaryContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &secondaryContainer();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &secondaryContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Secondary Fixed [QF]                                       //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_secondaryFixed{ Role::SecondaryFixed,
    {
        .name = "secondary_fixed",
        .isBackground = true,
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 80.0 : 90.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryFixed(), secondaryFixedDim(), 10.0, TonePolarity::Lighter, true };
            },
        },
        .spec2025 =
            ColourSpec{
                .palette = secondaryPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return secondaryContainer().getTone(s.asLightStandardContrast());
                    },
                .background = phoneHighestSurface,
                .contrastCurve = containerCurve2025,
            },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return secondaryContainer().getTone(s.asLightStandardContrast());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_secondaryFixedDim{ Role::SecondaryFixedDim,
    {
        .name = "secondary_fixed_dim",
        .isBackground = true,
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 70.0 : 80.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryFixed(), secondaryFixedDim(), 10.0, TonePolarity::Lighter, true };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return secondaryFixed().getTone(s);
                },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryFixedDim(), secondaryFixed(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Exact };
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return secondaryFixed().getTone(s);
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ secondaryFixedDim(), secondaryFixed(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Exact };
            },
        },
    } };

constexpr DynamicColour k_onSecondaryFixed{ Role::OnSecondaryFixed,
    {
        .name = "on_secondary_fixed",
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme&) {
                    return 10.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &secondaryFixedDim();
                },
            .secondBackground =
                [](const DynamicScheme&) {
                    return &secondaryFixed();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &secondaryFixedDim();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return fixedBackground2026(secondaryFixed().getTone(s), secondaryFixed(), secondaryFixedDim());
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
    } };

constexpr DynamicColour k_onSecondaryFixedVariant{ Role::OnSecondaryFixedVariant,
    {
        .name = "on_secondary_fixed_variant",
        .base = {
            .palette = secondaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 25.0 : 30.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &secondaryFixedDim();
                },
            .secondBackground =
                [](const DynamicScheme&) {
                    return &secondaryFixed();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &secondaryFixedDim();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
        },
        .spec2026 = ColourSpec{
            .palette = secondaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return fixedBackground2026(secondaryFixed().getTone(s), secondaryFixed(), secondaryFixedDim());
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Tertiaries [T]                                             //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_tertiary{ Role::Tertiary,
    {
        .name = "tertiary",
        .isBackground = true,
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 90.0 : 25.0;
                    return s.isDark() ? 80.0 : 40.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 7.0 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryContainer(), tertiary(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.tertiaryPalette();
                    if (!isPhone(s))
                        return s.variant() == Variant::TonalSpot ? tMaxC(palette, 0.0, 90.0) : tMaxC(palette);
                    if (s.variant() == Variant::Expressive || isVibrant(s)) {
                        const auto darkBound = s.isDark() ? 98.0 : 100.0;
                        return tMaxC(palette, 0.0, Hct::isCyan(palette.hue()) ? 88.0 : darkBound);
                    }
                    return s.isDark() ? tMaxC(palette, 0.0, 98.0) : tMaxC(palette);
                },
            .background = highestSurfaceOrContainerHigh,
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve4p5 : k_curve7;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ tertiaryContainer(), tertiary(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return secondarySourceColourHct(s).tone();
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ tertiaryContainer(), tertiary(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_tertiaryDim{ Role::TertiaryDim,
    {
        .name = "tertiary_dim",
        .isBackground = true,
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.tertiaryPalette();
                    return s.variant() == Variant::TonalSpot ? tMaxC(palette, 0.0, 90.0) : tMaxC(palette);
                },
            .background =
                [](const DynamicScheme&) {
                    return &surfaceContainerHigh();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryDim(), tertiary(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 =
            ColourSpec{
                .palette = tertiaryPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return tertiary().getTone(s);
                    },
            },
    } };

constexpr DynamicColour k_onTertiary{ Role::OnTertiary,
    {
        .name = "on_tertiary",
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 10.0 : 90.0;
                    return s.isDark() ? 20.0 : 100.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &tertiary();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return isPhone(s) ? &tertiary() : &tertiaryDim();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &tertiary();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

constexpr DynamicColour k_tertiaryContainer{ Role::TertiaryContainer,
    {
        .name = "tertiary_container",
        .isBackground = true,
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 60.0 : 49.0;
                    if (!isFidelity(s))
                        return s.isDark() ? 30.0 : 90.0;
                    return dislike::fixIfDisliked(s.tertiaryPalette().getHct(s.sourceColourHct().tone())).tone();
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryContainer(), tertiary(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.tertiaryPalette();
                    if (!isPhone(s))
                        return s.variant() == Variant::TonalSpot ? tMaxC(palette, 0.0, 90.0) : tMaxC(palette);
                    switch (s.variant()) {
                    case Variant::Neutral:
                        return s.isDark() ? tMaxC(palette, 0.0, 93.0) : tMaxC(palette, 0.0, 96.0);
                    case Variant::TonalSpot:
                        return tMaxC(palette, 0.0, s.isDark() ? 93.0 : 100.0);
                    case Variant::Expressive: {
                        const auto darkBound = s.isDark() ? 93.0 : 100.0;
                        return tMaxC(palette, 75.0, Hct::isCyan(palette.hue()) ? 88.0 : darkBound);
                    }
                    default:
                        return s.isDark() ? tMaxC(palette, 0.0, 93.0) : tMaxC(palette, 72.0, 100.0);
                    }
                },
            .background = phoneHighestSurface,
            .contrastCurve = containerCurve2025,
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ tertiaryContainer(), tertiaryDim(), 10.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto tone = secondarySourceColourHct(s).tone();
                    return tone > 55.0 ? std::clamp(tone, 61.0, 90.0) : std::clamp(tone, 20.0, 49.0);
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

constexpr DynamicColour k_onTertiaryContainer{ Role::OnTertiaryContainer,
    {
        .name = "on_tertiary_container",
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 0.0 : 100.0;
                    if (!isFidelity(s))
                        return s.isDark() ? 90.0 : 30.0;
                    return DynamicColour::foregroundTone(tertiaryContainer().tone(s), 4.5);
                },
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryContainer();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Tertiary Fixed [TF]                                        //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_tertiaryFixed{ Role::TertiaryFixed,
    {
        .name = "tertiary_fixed",
        .isBackground = true,
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 40.0 : 90.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryFixed(), tertiaryFixedDim(), 10.0, TonePolarity::Lighter, true };
            },
        },
        .spec2025 =
            ColourSpec{
                .palette = tertiaryPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return tertiaryContainer().getTone(s.asLightStandardContrast());
                    },
                .background = phoneHighestSurface,
                .contrastCurve = containerCurve2025,
            },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return tertiaryContainer().getTone(s.asLightStandardContrast());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_tertiaryFixedDim{ Role::TertiaryFixedDim,
    {
        .name = "tertiary_fixed_dim",
        .isBackground = true,
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 30.0 : 80.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryFixed(), tertiaryFixedDim(), 10.0, TonePolarity::Lighter, true };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return tertiaryFixed().getTone(s);
                },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryFixedDim(), tertiaryFixed(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Exact };
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return tertiaryFixed().getTone(s);
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ tertiaryFixedDim(), tertiaryFixed(), 5.0, TonePolarity::Darker, true,
                    DeltaConstraint::Exact };
            },
        },
    } };

constexpr DynamicColour k_onTertiaryFixed{ Role::OnTertiaryFixed,
    {
        .name = "on_tertiary_fixed",
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 100.0 : 10.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryFixedDim();
                },
            .secondBackground =
                [](const DynamicScheme&) {
                    return &tertiaryFixed();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryFixedDim();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return fixedBackground2026(tertiaryFixed().getTone(s), tertiaryFixed(), tertiaryFixedDim());
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve7;
            },
        },
    } };

constexpr DynamicColour k_onTertiaryFixedVariant{ Role::OnTertiaryFixedVariant,
    {
        .name = "on_tertiary_fixed_variant",
        .base = {
            .palette = tertiaryPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return isMonochrome(s) ? 90.0 : 30.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryFixedDim();
                },
            .secondBackground =
                [](const DynamicScheme&) {
                    return &tertiaryFixed();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme&) {
                    return &tertiaryFixedDim();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
        },
        .spec2026 = ColourSpec{
            .palette = tertiaryPalette,
            .background =
                [](const DynamicScheme& s) {
                    return fixedBackground2026(tertiaryFixed().getTone(s), tertiaryFixed(), tertiaryFixedDim());
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Errors [E]                                                 //
////////////////////////////////////////////////////////////////

constexpr DynamicColour k_error{ Role::Error,
    {
        .name = "error",
        .isBackground = true,
        .base = {
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 80.0 : 40.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 7.0 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ errorContainer(), error(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.errorPalette();
                    if (!isPhone(s))
                        return tMinC(palette);
                    return s.isDark() ? tMinC(palette, 0.0, 98.0) : tMaxC(palette);
                },
            .background = highestSurfaceOrContainerHigh,
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve4p5 : k_curve7;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ errorContainer(), error(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return tMaxC(s.errorPalette());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (!isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ errorContainer(), error(), 5.0, TonePolarity::RelativeLighter, true,
                    DeltaConstraint::Farther };
            },
        },
    } };

// NOLINTNEXTLINE(misc-static-initialization-cycle): lambdas run after initialisation
constexpr DynamicColour k_errorDim{ Role::ErrorDim,
    {
        .name = "error_dim",
        .isBackground = true,
        .base = {
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return tMinC(s.errorPalette());
                },
            .background =
                [](const DynamicScheme&) {
                    return &surfaceContainerHigh();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve4p5;
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ errorDim(), error(), 5.0, TonePolarity::Darker, true, DeltaConstraint::Farther };
            },
        },
        .spec2026 =
            ColourSpec{
                .palette = errorPalette,
                .tone =
                    [](const DynamicScheme& s) {
                        return error().getTone(s);
                    },
            },
    } };

constexpr DynamicColour k_onError{ Role::OnError,
    {
        .name = "on_error",
        .base = {
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 20.0 : 100.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &error();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 4.5, 7.0, 11.0, 21.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = errorPalette,
            .background =
                [](const DynamicScheme& s) {
                    return isPhone(s) ? &error() : &errorDim();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve6 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = errorPalette,
            .background =
                [](const DynamicScheme&) {
                    return &error();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

constexpr DynamicColour k_errorContainer{ Role::ErrorContainer,
    {
        .name = "error_container",
        .isBackground = true,
        .base = {
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? 30.0 : 90.0;
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 1.0, 1.0, 3.0, 4.5 };
            },
            .toneDeltaPair = [](const DynamicScheme&) -> std::optional<ToneDeltaPair> {
                return ToneDeltaPair{ errorContainer(), error(), 10.0, TonePolarity::Nearer, false };
            },
        },
        .spec2025 = ColourSpec{
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    const auto& palette = s.errorPalette();
                    if (!isPhone(s))
                        return 30.0;
                    return s.isDark() ? tMinC(palette, 30.0, 93.0) : tMaxC(palette, 0.0, 90.0);
                },
            .background = phoneHighestSurface,
            .contrastCurve = containerCurve2025,
            .toneDeltaPair = [](const DynamicScheme& s) -> std::optional<ToneDeltaPair> {
                if (isPhone(s))
                    return std::nullopt;
                return ToneDeltaPair{ errorContainer(), errorDim(), 10.0, TonePolarity::Darker, true,
                    DeltaConstraint::Farther };
            },
        },
        .spec2026 = ColourSpec{
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    return s.isDark() ? tMinC(s.errorPalette()) : tMaxC(s.errorPalette());
                },
            .background =
                [](const DynamicScheme& s) {
                    return &highestSurface(s);
                },
            .contrastCurve = containerCurve2026,
        },
    } };

constexpr DynamicColour k_onErrorContainer{ Role::OnErrorContainer,
    {
        .name = "on_error_container",
        .base = {
            .palette = errorPalette,
            .tone =
                [](const DynamicScheme& s) {
                    if (isMonochrome(s))
                        return s.isDark() ? 90.0 : 10.0;
                    return s.isDark() ? 90.0 : 30.0;
                },
            .background =
                [](const DynamicScheme&) {
                    return &errorContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return ContrastCurve{ 3.0, 4.5, 7.0, 11.0 };
            },
        },
        .spec2025 = ColourSpec{
            .palette = errorPalette,
            .background =
                [](const DynamicScheme&) {
                    return &errorContainer();
                },
            .contrastCurve = [](const DynamicScheme& s) -> std::optional<ContrastCurve> {
                return isPhone(s) ? k_curve4p5 : k_curve7;
            },
        },
        .spec2026 = ColourSpec{
            .palette = errorPalette,
            .background =
                [](const DynamicScheme&) {
                    return &errorContainer();
                },
            .contrastCurve = [](const DynamicScheme&) -> std::optional<ContrastCurve> {
                return k_curve6;
            },
        },
    } };

////////////////////////////////////////////////////////////////
// Accessors                                                  //
////////////////////////////////////////////////////////////////

// clang-format off
const DynamicColour& primaryPaletteKeyColour() { return k_primaryPaletteKeyColour; }
const DynamicColour& secondaryPaletteKeyColour() { return k_secondaryPaletteKeyColour; }
const DynamicColour& tertiaryPaletteKeyColour() { return k_tertiaryPaletteKeyColour; }
const DynamicColour& neutralPaletteKeyColour() { return k_neutralPaletteKeyColour; }
const DynamicColour& neutralVariantPaletteKeyColour() { return k_neutralVariantPaletteKeyColour; }
const DynamicColour& errorPaletteKeyColour() { return k_errorPaletteKeyColour; }
const DynamicColour& background() { return k_background; }
const DynamicColour& onBackground() { return k_onBackground; }
const DynamicColour& surface() { return k_surface; }
const DynamicColour& surfaceDim() { return k_surfaceDim; }
const DynamicColour& surfaceBright() { return k_surfaceBright; }
const DynamicColour& surfaceContainerLowest() { return k_surfaceContainerLowest; }
const DynamicColour& surfaceContainerLow() { return k_surfaceContainerLow; }
const DynamicColour& surfaceContainer() { return k_surfaceContainer; }
const DynamicColour& surfaceContainerHigh() { return k_surfaceContainerHigh; }
const DynamicColour& surfaceContainerHighest() { return k_surfaceContainerHighest; }
const DynamicColour& onSurface() { return k_onSurface; }
const DynamicColour& surfaceVariant() { return k_surfaceVariant; }
const DynamicColour& onSurfaceVariant() { return k_onSurfaceVariant; }
const DynamicColour& inverseSurface() { return k_inverseSurface; }
const DynamicColour& inverseOnSurface() { return k_inverseOnSurface; }
const DynamicColour& outline() { return k_outline; }
const DynamicColour& outlineVariant() { return k_outlineVariant; }
const DynamicColour& shadow() { return k_shadow; }
const DynamicColour& scrim() { return k_scrim; }
const DynamicColour& surfaceTint() { return k_surfaceTint; }
const DynamicColour& primary() { return k_primary; }
const DynamicColour& primaryDim() { return k_primaryDim; }
const DynamicColour& onPrimary() { return k_onPrimary; }
const DynamicColour& primaryContainer() { return k_primaryContainer; }
const DynamicColour& onPrimaryContainer() { return k_onPrimaryContainer; }
const DynamicColour& inversePrimary() { return k_inversePrimary; }
const DynamicColour& primaryFixed() { return k_primaryFixed; }
const DynamicColour& primaryFixedDim() { return k_primaryFixedDim; }
const DynamicColour& onPrimaryFixed() { return k_onPrimaryFixed; }
const DynamicColour& onPrimaryFixedVariant() { return k_onPrimaryFixedVariant; }
const DynamicColour& secondary() { return k_secondary; }
const DynamicColour& secondaryDim() { return k_secondaryDim; }
const DynamicColour& onSecondary() { return k_onSecondary; }
const DynamicColour& secondaryContainer() { return k_secondaryContainer; }
const DynamicColour& onSecondaryContainer() { return k_onSecondaryContainer; }
const DynamicColour& secondaryFixed() { return k_secondaryFixed; }
const DynamicColour& secondaryFixedDim() { return k_secondaryFixedDim; }
const DynamicColour& onSecondaryFixed() { return k_onSecondaryFixed; }
const DynamicColour& onSecondaryFixedVariant() { return k_onSecondaryFixedVariant; }
const DynamicColour& tertiary() { return k_tertiary; }
const DynamicColour& tertiaryDim() { return k_tertiaryDim; }
const DynamicColour& onTertiary() { return k_onTertiary; }
const DynamicColour& tertiaryContainer() { return k_tertiaryContainer; }
const DynamicColour& onTertiaryContainer() { return k_onTertiaryContainer; }
const DynamicColour& tertiaryFixed() { return k_tertiaryFixed; }
const DynamicColour& tertiaryFixedDim() { return k_tertiaryFixedDim; }
const DynamicColour& onTertiaryFixed() { return k_onTertiaryFixed; }
const DynamicColour& onTertiaryFixedVariant() { return k_onTertiaryFixedVariant; }
const DynamicColour& error() { return k_error; }
const DynamicColour& errorDim() { return k_errorDim; }
const DynamicColour& onError() { return k_onError; }
const DynamicColour& errorContainer() { return k_errorContainer; }
const DynamicColour& onErrorContainer() { return k_onErrorContainer; }
// clang-format on

////////////////////////////////////////////////////////////////
// All Colours                                                //
////////////////////////////////////////////////////////////////

std::span<const DynamicColour* const> allColours() {
    static constexpr std::array k_colours{
        &k_background,
        &k_onBackground,
        &k_surface,
        &k_surfaceDim,
        &k_surfaceBright,
        &k_surfaceContainerLowest,
        &k_surfaceContainerLow,
        &k_surfaceContainer,
        &k_surfaceContainerHigh,
        &k_surfaceContainerHighest,
        &k_onSurface,
        &k_onSurfaceVariant,
        &k_outline,
        &k_outlineVariant,
        &k_inverseSurface,
        &k_inverseOnSurface,
        &k_primary,
        &k_primaryDim,
        &k_onPrimary,
        &k_primaryContainer,
        &k_onPrimaryContainer,
        &k_primaryFixed,
        &k_primaryFixedDim,
        &k_onPrimaryFixed,
        &k_onPrimaryFixedVariant,
        &k_inversePrimary,
        &k_secondary,
        &k_secondaryDim,
        &k_onSecondary,
        &k_secondaryContainer,
        &k_onSecondaryContainer,
        &k_secondaryFixed,
        &k_secondaryFixedDim,
        &k_onSecondaryFixed,
        &k_onSecondaryFixedVariant,
        &k_tertiary,
        &k_tertiaryDim,
        &k_onTertiary,
        &k_tertiaryContainer,
        &k_onTertiaryContainer,
        &k_tertiaryFixed,
        &k_tertiaryFixedDim,
        &k_onTertiaryFixed,
        &k_onTertiaryFixedVariant,
        &k_error,
        &k_errorDim,
        &k_onError,
        &k_errorContainer,
        &k_onErrorContainer,
    };
    return k_colours;
}

} // namespace m3colours::dynamiccolour::material
