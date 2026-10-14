#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace m3colours {

namespace detail {

constexpr std::uint8_t delinearise(double rgbComponent) {
    const auto normalised = rgbComponent / 100.0;
    const auto delinearised = normalised <= 0.0031308 // format hack
                                  ? normalised * 12.92
                                  : 1.055 * std::pow(normalised, 1.0 / 2.4) - 0.055;
    const auto scaled = static_cast<int>(std::round(delinearised * 255.0));
    return static_cast<std::uint8_t>(std::clamp(scaled, 0, 255));
}

constexpr double linearise(const int rgbComponent) {
    const auto normalised = rgbComponent / 255.0;
    if (normalised <= 0.040449936)
        return normalised / 12.92 * 100.0;
    return std::pow((normalised + 0.055) / 1.055, 2.4) * 100.0;
}

} // namespace detail

struct Vec3 {
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
};

class Argb {
public:
    constexpr explicit Argb(std::uint32_t value)
        : m_value(value) {}

    constexpr explicit Argb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
        : m_value(0xFF000000u | (std::uint32_t{ r } << 16u) | (std::uint32_t{ g } << 8u) | b) {}

    constexpr explicit Argb(Vec3 linearRgb)
        : Argb(detail::delinearise(linearRgb.a), detail::delinearise(linearRgb.b), detail::delinearise(linearRgb.c)) {}

    [[nodiscard]] constexpr std::uint32_t value() const { return m_value; }

    // clang-format off
    [[nodiscard]] constexpr std::uint8_t red() const { return (m_value & 0x00ff0000) >> 16; }
    [[nodiscard]] constexpr std::uint8_t green() const { return (m_value & 0x0000ff00) >> 8; }
    [[nodiscard]] constexpr std::uint8_t blue() const { return m_value & 0x000000ff; }
    [[nodiscard]] constexpr std::uint8_t alpha() const { return (m_value & 0xff000000) >> 24; }
    // clang-format on

    [[nodiscard]] constexpr bool isOpaque() const { return alpha() == 255; }

private:
    std::uint32_t m_value;
};

namespace detail {

constexpr double lstarFromY(double y) {
    const auto yNormalized = y / 100.0;
    if (yNormalized <= 216.0 / 24389.0)
        return (24389.0 / 27.0) * yNormalized;
    return 116.0 * std::pow(yNormalized, 1.0 / 3.0) - 16.0;
}

constexpr double argbToY(Argb argb) {
    const auto redL = linearise(argb.red());
    const auto greenL = linearise(argb.green());
    const auto blueL = linearise(argb.blue());
    return 0.2126 * redL + 0.7152 * greenL + 0.0722 * blueL;
}

} // namespace detail

class Lstar {
public:
    constexpr explicit Lstar(double y)
        : m_value(detail::lstarFromY(y)) {}

    constexpr explicit Lstar(Argb argb)
        : Lstar(detail::argbToY(argb)) {}

    [[nodiscard]] constexpr double value() const { return m_value; }

    [[nodiscard]] constexpr double y() const {
        if (m_value > 8.0) {
            double cubeRoot = (m_value + 16.0) / 116.0;
            double cube = cubeRoot * cubeRoot * cubeRoot;
            return cube * 100.0;
        }
        return m_value / (24389.0 / 27.0) * 100.0;
    }

private:
    double m_value;
};

} // namespace m3colours
