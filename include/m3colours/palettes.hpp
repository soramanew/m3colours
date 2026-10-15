#pragma once

#include <optional>
#include <unordered_map>

#include "common.hpp"
#include "hct.hpp"

namespace m3colours::palettes {

class TonalPalette {
public:
    static TonalPalette fromArgb(Argb argb);
    static TonalPalette fromHct(const hct::Hct& hct);
    static TonalPalette fromHueAndChroma(double hue, double chroma);

    // clang-format off
    [[nodiscard]] double hue() const { return m_hue; }
    [[nodiscard]] double chroma() const { return m_chroma; }
    [[nodiscard]] const hct::Hct& keyColour() const { return m_keyColour; }
    // clang-format on

    [[nodiscard]] Argb tone(double tone) const;
    [[nodiscard]] hct::Hct getHct(double tone) const;

private:
    TonalPalette(double hue, double chroma, hct::Hct keyColour);

    double m_hue;
    double m_chroma;
    hct::Hct m_keyColour;
    mutable std::unordered_map<double, Argb> m_cache;
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
