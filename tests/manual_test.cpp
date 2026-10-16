#include <algorithm>
#include <charconv>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>

#include "m3colours/common.hpp"
#include "m3colours/dynamiccolour.hpp"
#include "m3colours/quantise.hpp"
#include "m3colours/scheme.hpp"
#include "m3colours/score.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SWATCH_FORMAT "\x1b[48;2;%u;%u;%um   \x1b[0m"

namespace {

constexpr std::array k_allVariants = {
    "monochrome",
    "neutral",
    "tonalspot",
    "vibrant",
    "expressive",
    "fidelity",
    "content",
    "rainbow",
    "fruitsalad",
    "cmf",
};

enum class PrintFlags : std::uint8_t {
    Default = 0,
    All = 1 << 0,
    Quantised = 1 << 1,
    Score = 1 << 2,
    SchemeSingle = 1 << 3,
    SchemeAll = 1 << 4,
};

PrintFlags& operator|=(PrintFlags& lhs, PrintFlags rhs) {
    using T = std::underlying_type_t<PrintFlags>;
    lhs = static_cast<PrintFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
    return lhs;
}

PrintFlags operator&(PrintFlags lhs, PrintFlags rhs) {
    using T = std::underlying_type_t<PrintFlags>;
    return static_cast<PrintFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

enum class UsageCause : std::uint8_t {
    None,
    Help,
    Other,
};

struct Args {
    UsageCause needsUsage = UsageCause::None;
    std::optional<std::string> invalidBecause;
    std::optional<std::string> imgPath;
    std::optional<m3colours::Argb> sourceColour;
    std::string variant = "cmf";
    bool dark = false;
    PrintFlags printFlags = PrintFlags::Default;
};

PrintFlags parsePrintFlags(const std::string& str) {
    PrintFlags flags = PrintFlags::Default;
    for (const auto& c : str | std::views::split(',')) {
        const auto chunk = std::string_view(c.begin(), c.end());
        if (chunk == "all")
            flags |= PrintFlags::All;
        else if (chunk == "quantised")
            flags |= PrintFlags::Quantised;
        else if (chunk == "score")
            flags |= PrintFlags::Score;
        else if (chunk == "scheme-single")
            flags |= PrintFlags::SchemeSingle;
        else if (chunk == "scheme" || chunk == "scheme-all")
            flags |= PrintFlags::SchemeAll;
    }

    return flags;
}

Args parseArgs(int argc, char* argv[]) {
    Args args;

    std::vector<std::string> argStrs(argv + 1, argv + argc);

    for (std::size_t i = 0; i < argStrs.size(); ++i) {
        const auto& arg = argStrs[i];

        if (arg == "--image") {
            args.imgPath = argStrs[++i];
        } else if (arg == "--source-colour") {
            std::string_view col = argStrs[++i];
            if (col.starts_with('#'))
                col.remove_prefix(1);
            else if (col.starts_with("0x"))
                col.remove_prefix(2);

            uint32_t argb = 0;
            const auto [p, ec] = std::from_chars(col.data(), col.data() + col.size(), argb, 16);
            if (ec == std::errc::result_out_of_range || ec == std::errc::invalid_argument || p == col.data()) {
                args.invalidBecause = "invalid source colour";
                return args;
            }

            args.sourceColour = m3colours::Argb{ argb };
        } else if (arg == "--variant") {
            const auto& variant = argStrs[++i];
            if (std::ranges::find(k_allVariants, variant) == k_allVariants.end()) {
                args.invalidBecause = "invalid variant";
                return args;
            }

            args.variant = variant;
        } else if (arg == "--dark") {
            args.dark = true;
        } else if (arg == "--print") {
            args.printFlags = parsePrintFlags(argStrs[++i]);
        } else {
            args.needsUsage = arg == "-h" || arg == "--help" ? UsageCause::Help : UsageCause::Other;
        }
    }

    if (argStrs.empty())
        args.needsUsage = UsageCause::Other;

    return args;
}

std::optional<m3colours::quantise::QuantiserResult> quantiseImage(const Args& args) {
    int w;
    int h;
    auto* const data = stbi_load(args.imgPath->c_str(), &w, &h, nullptr, 4);
    const auto width = static_cast<std::size_t>(w);
    const auto height = static_cast<std::size_t>(h);

    if (data == nullptr) {
        std::cerr << "Failed to load image\n";
        return {};
    }

    std::vector<m3colours::Argb> pixels;
    pixels.reserve(width * height);
    for (std::size_t i = 0; i < width * height; ++i) {
        const auto r = data[i * 4];
        const auto g = data[i * 4 + 1];
        const auto b = data[i * 4 + 2];
        const auto a = data[i * 4 + 3];

        pixels.emplace_back((a << 24) | (r << 16) | (g << 8) | b);
    }

    auto res = m3colours::quantise::celebi(pixels, 128);
    stbi_image_free(data);
    return res;
}

m3colours::scheme::DynamicScheme schemeForVariant(m3colours::Hct source, const std::string& variant, bool dark) {
    if (variant == "monochrome")
        return m3colours::scheme::SchemeMonochrome(source, dark, 0);
    if (variant == "neutral")
        return m3colours::scheme::SchemeNeutral(source, dark, 0);
    if (variant == "tonalspot")
        return m3colours::scheme::SchemeTonalSpot(source, dark, 0);
    if (variant == "vibrant")
        return m3colours::scheme::SchemeVibrant(source, dark, 0);
    if (variant == "expressive")
        return m3colours::scheme::SchemeExpressive(source, dark, 0);
    if (variant == "fidelity")
        return m3colours::scheme::SchemeFidelity(source, dark, 0);
    if (variant == "content")
        return m3colours::scheme::SchemeContent(source, dark, 0);
    if (variant == "rainbow")
        return m3colours::scheme::SchemeRainbow(source, dark, 0);
    if (variant == "fruitsalad")
        return m3colours::scheme::SchemeFruitSalad(source, dark, 0);

    if (variant != "cmf")
        std::cerr << "Got unknown variant, something is wrong\n";

    return m3colours::scheme::SchemeCmf(source, dark, 0);
}

void printScored(const std::vector<m3colours::Argb>& scored) {
    for (std::size_t i = 0; i < scored.size(); ++i) {
        const auto c = scored[i];
        std::printf("%zu. %06x " SWATCH_FORMAT "\n", i + 1, c.value() & 0xffffffu, c.red(), c.green(), c.blue());
    }
}

void printVariant(m3colours::Hct source, const std::string& variant, bool dark) {
    std::cout << "\n-- Scheme (" << variant << ") --\n\n";
    const auto scheme = schemeForVariant(source, variant, dark);
    for (const auto* colour : m3colours::dynamiccolour::material::allColours()) {
        const auto argb = scheme.getArgb(*colour);
        std::printf("%-32s %06x " SWATCH_FORMAT "\n", colour->name().c_str(), argb.value() & 0xffffffu, argb.red(),
            argb.green(), argb.blue());
    }
}

void printScheme(m3colours::Argb source, const Args& args) {
    m3colours::Hct hct{ source };

    if ((args.printFlags & PrintFlags::SchemeAll) == PrintFlags::SchemeAll) {
        for (const auto* variant : k_allVariants)
            printVariant(hct, variant, args.dark);
    } else if (args.printFlags == PrintFlags::Default ||
               (args.printFlags & PrintFlags::SchemeSingle) == PrintFlags::SchemeSingle) {
        printVariant(hct, args.variant, args.dark);
    }
}

} // namespace

int main(int argc, char* argv[]) {
    const auto args = parseArgs(argc, argv);
    if (args.invalidBecause) {
        std::cerr << "Failed to parse args: " << *args.invalidBecause << '\n';
        return 1;
    }

    if (args.needsUsage != UsageCause::None) {
        std::cerr << "Usage: " << argv[0]
                  << " [-h|--help] [--image <path>] [--source-colour <argb>]"
                     " [--variant <variant>] [--dark] [--print <parts,..>]\n";
        return args.needsUsage == UsageCause::Help ? 0 : 1;
    }

    if (args.imgPath) {
        std::cout << "\n=== From image ===\n\n";
        std::cout << "Image: " << *args.imgPath << '\n';

        const auto res = quantiseImage(args);
        if (!res)
            return 1;

        if ((args.printFlags & PrintFlags::Quantised) == PrintFlags::Quantised) {
            std::cout << "\n-- Quantised --\n\n";
            std::vector<std::pair<m3colours::Argb, std::uint32_t>> entries(
                res->colourToCount.begin(), res->colourToCount.end());
            std::ranges::sort(entries, std::ranges::greater{}, &decltype(entries)::value_type::second);
            for (const auto& [colour, count] : entries)
                std::printf("%06x: %u\n", colour.value() & 0xffffffu, count);
        }

        const auto ranked = m3colours::score::score(res->colourToCount);

        if (args.printFlags == PrintFlags::Default || (args.printFlags & PrintFlags::Score) == PrintFlags::Score) {
            std::cout << "\n-- Score (default) --\n\n";
            printScored(ranked);
            std::cout << "\n-- Score (cutoff) --\n\n";
            printScored(m3colours::score::scoreByCutoff(res->colourToCount));
        }

        printScheme(ranked[0], args);
    }

    if (args.sourceColour) {
        std::cout << "\n=== From source colour ===\n\n";
        printScheme(*args.sourceColour, args);
    }

    return 0;
}
