#include "m3colours/scheme.hpp"

namespace m3colours::scheme {

using palettes::CorePalette;

Scheme Scheme::light(Argb argb) {
    return lightFromCorePalette(CorePalette::of(argb));
}

Scheme Scheme::dark(Argb argb) {
    return darkFromCorePalette(CorePalette::of(argb));
}

Scheme Scheme::lightContent(Argb argb) {
    return lightFromCorePalette(CorePalette::contentOf(argb));
}

Scheme Scheme::darkContent(Argb argb) {
    return darkFromCorePalette(CorePalette::contentOf(argb));
}

Scheme Scheme::lightFromCorePalette(const CorePalette& core) {
    return {
        .primary = core.a1.tone(40),
        .onPrimary = core.a1.tone(100),
        .primaryContainer = core.a1.tone(90),
        .onPrimaryContainer = core.a1.tone(10),
        .secondary = core.a2.tone(40),
        .onSecondary = core.a2.tone(100),
        .secondaryContainer = core.a2.tone(90),
        .onSecondaryContainer = core.a2.tone(10),
        .tertiary = core.a3.tone(40),
        .onTertiary = core.a3.tone(100),
        .tertiaryContainer = core.a3.tone(90),
        .onTertiaryContainer = core.a3.tone(10),
        .error = core.error.tone(40),
        .onError = core.error.tone(100),
        .errorContainer = core.error.tone(90),
        .onErrorContainer = core.error.tone(10),
        .background = core.n1.tone(99),
        .onBackground = core.n1.tone(10),
        .surface = core.n1.tone(99),
        .onSurface = core.n1.tone(10),
        .surfaceVariant = core.n2.tone(90),
        .onSurfaceVariant = core.n2.tone(30),
        .outline = core.n2.tone(50),
        .outlineVariant = core.n2.tone(80),
        .shadow = core.n1.tone(0),
        .scrim = core.n1.tone(0),
        .inverseSurface = core.n1.tone(20),
        .inverseOnSurface = core.n1.tone(95),
        .inversePrimary = core.a1.tone(80),
    };
}

Scheme Scheme::darkFromCorePalette(const CorePalette& core) {
    return {
        .primary = core.a1.tone(80),
        .onPrimary = core.a1.tone(20),
        .primaryContainer = core.a1.tone(30),
        .onPrimaryContainer = core.a1.tone(90),
        .secondary = core.a2.tone(80),
        .onSecondary = core.a2.tone(20),
        .secondaryContainer = core.a2.tone(30),
        .onSecondaryContainer = core.a2.tone(90),
        .tertiary = core.a3.tone(80),
        .onTertiary = core.a3.tone(20),
        .tertiaryContainer = core.a3.tone(30),
        .onTertiaryContainer = core.a3.tone(90),
        .error = core.error.tone(80),
        .onError = core.error.tone(20),
        .errorContainer = core.error.tone(30),
        .onErrorContainer = core.error.tone(80),
        .background = core.n1.tone(10),
        .onBackground = core.n1.tone(90),
        .surface = core.n1.tone(10),
        .onSurface = core.n1.tone(90),
        .surfaceVariant = core.n2.tone(30),
        .onSurfaceVariant = core.n2.tone(80),
        .outline = core.n2.tone(60),
        .outlineVariant = core.n2.tone(30),
        .shadow = core.n1.tone(0),
        .scrim = core.n1.tone(0),
        .inverseSurface = core.n1.tone(90),
        .inverseOnSurface = core.n1.tone(20),
        .inversePrimary = core.a1.tone(40),
    };
}

} // namespace m3colours::scheme
