#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "common.hpp"
#include "hct.hpp"

namespace m3colours::palettes {

class TonalPalette {
public:
    static TonalPalette fromArgb(Argb argb);
    static TonalPalette fromHct(const Hct& hct);
    static TonalPalette fromHueAndChroma(double hue, double chroma);

    // clang-format off
    [[nodiscard]] double hue() const { return m_hue; }
    [[nodiscard]] double chroma() const { return m_chroma; }
    // clang-format on

    [[nodiscard]] const Hct& keyColour() const;
    [[nodiscard]] Argb tone(double tone) const;
    [[nodiscard]] Hct getHct(double tone) const;
    [[nodiscard]] double toneOfMaxChroma() const;
    [[nodiscard]] double toneOfMinChroma() const;

private:
    TonalPalette(double hue, double chroma, std::optional<Hct> keyColour);

    double m_hue;
    double m_chroma;
    mutable std::optional<Hct> m_keyColour;
    mutable std::optional<double> m_toneOfMaxChroma;
    mutable std::optional<double> m_toneOfMinChroma;
    mutable std::array<std::uint32_t, 101> m_toneCache{};
};

struct CorePalettes {
    TonalPalette primary;
    TonalPalette secondary;
    TonalPalette tertiary;
    TonalPalette neutral;
    TonalPalette neutralVariant;
};

struct CorePaletteColours {
    Argb primary;
    std::optional<Argb> secondary;
    std::optional<Argb> tertiary;
    std::optional<Argb> neutral;
    std::optional<Argb> neutralVariant;
    std::optional<Argb> error;
};

class CorePalette {
public:
    static CorePalette of(Argb argb);
    static CorePalette contentOf(Argb argb);
    static CorePalette fromColours(const CorePaletteColours& colours);
    static CorePalette contentFromColours(const CorePaletteColours& colours);

    TonalPalette a1;
    TonalPalette a2;
    TonalPalette a3;
    TonalPalette n1;
    TonalPalette n2;
    TonalPalette error;

private:
    CorePalette(double hue, double chroma, bool isContent);
};

} // namespace m3colours::palettes
