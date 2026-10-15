#include "m3colours/scheme.hpp"

namespace m3colours::scheme {

using palettes::CorePalette;

SchemeAndroid SchemeAndroid::light(Argb argb) {
    return lightFromCorePalette(CorePalette::of(argb));
}

SchemeAndroid SchemeAndroid::dark(Argb argb) {
    return darkFromCorePalette(CorePalette::of(argb));
}

SchemeAndroid SchemeAndroid::lightContent(Argb argb) {
    return lightFromCorePalette(CorePalette::contentOf(argb));
}

SchemeAndroid SchemeAndroid::darkContent(Argb argb) {
    return darkFromCorePalette(CorePalette::contentOf(argb));
}

SchemeAndroid SchemeAndroid::lightFromCorePalette(const CorePalette& core) {
    return {
        .colourAccentPrimary = core.a1.tone(90),
        .colourAccentPrimaryVariant = core.a1.tone(40),
        .colourAccentSecondary = core.a2.tone(90),
        .colourAccentSecondaryVariant = core.a2.tone(40),
        .colourAccentTertiary = core.a3.tone(90),
        .colourAccentTertiaryVariant = core.a3.tone(40),
        .textColourPrimary = core.n1.tone(10),
        .textColourSecondary = core.n2.tone(30),
        .textColourTertiary = core.n2.tone(50),
        .textColourPrimaryInverse = core.n1.tone(95),
        .textColourSecondaryInverse = core.n1.tone(80),
        .textColourTertiaryInverse = core.n1.tone(60),
        .colourBackground = core.n1.tone(95),
        .colourBackgroundFloating = core.n1.tone(98),
        .colourSurface = core.n1.tone(98),
        .colourSurfaceVariant = core.n1.tone(90),
        .colourSurfaceHighlight = core.n1.tone(100),
        .surfaceHeader = core.n1.tone(90),
        .underSurface = core.n1.tone(0),
        .offState = core.n1.tone(20),
        .accentSurface = core.a2.tone(95),
        .textPrimaryOnAccent = core.n1.tone(10),
        .textSecondaryOnAccent = core.n2.tone(30),
        .volumeBackground = core.n1.tone(25),
        .scrim = core.n1.tone(80),
    };
}

SchemeAndroid SchemeAndroid::darkFromCorePalette(const CorePalette& core) {
    return {
        .colourAccentPrimary = core.a1.tone(90),
        .colourAccentPrimaryVariant = core.a1.tone(70),
        .colourAccentSecondary = core.a2.tone(90),
        .colourAccentSecondaryVariant = core.a2.tone(70),
        .colourAccentTertiary = core.a3.tone(90),
        .colourAccentTertiaryVariant = core.a3.tone(70),
        .textColourPrimary = core.n1.tone(95),
        .textColourSecondary = core.n2.tone(80),
        .textColourTertiary = core.n2.tone(60),
        .textColourPrimaryInverse = core.n1.tone(10),
        .textColourSecondaryInverse = core.n1.tone(30),
        .textColourTertiaryInverse = core.n1.tone(50),
        .colourBackground = core.n1.tone(10),
        .colourBackgroundFloating = core.n1.tone(10),
        .colourSurface = core.n1.tone(20),
        .colourSurfaceVariant = core.n1.tone(30),
        .colourSurfaceHighlight = core.n1.tone(35),
        .surfaceHeader = core.n1.tone(30),
        .underSurface = core.n1.tone(0),
        .offState = core.n1.tone(20),
        .accentSurface = core.a2.tone(95),
        .textPrimaryOnAccent = core.n1.tone(10),
        .textSecondaryOnAccent = core.n2.tone(30),
        .volumeBackground = core.n1.tone(25),
        .scrim = core.n1.tone(80),
    };
}

} // namespace m3colours::scheme
