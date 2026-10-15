#include "m3colours/score.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>

#include "m3colours/hct.hpp"

namespace m3colours::score {

namespace {

constexpr double k_targetChroma = 48.0;
constexpr double k_weightProportion = 0.7;
constexpr double k_weightChromaAbove = 0.3;
constexpr double k_weightChromaBelow = 0.1;
constexpr double k_cutoffChroma = 5.0;
constexpr double k_cutoffExcitedProportion = 0.01;

struct ScoredHct {
    Hct hct;
    double score;
};

constexpr std::size_t sanitiseDegreesInt(int degrees) {
    degrees %= 360;
    return static_cast<std::size_t>(degrees < 0 ? degrees + 360 : degrees);
}

std::vector<ScoredHct> scoreColours(const std::map<Argb, std::uint32_t>& coloursToPopulation, bool filter) {
    std::vector<Hct> coloursHct;
    coloursHct.reserve(coloursToPopulation.size());
    std::array<double, 360> huePopulation{};
    double populationSum = 0.0;
    for (const auto& [argb, population] : coloursToPopulation) {
        const auto& hct = coloursHct.emplace_back(argb);
        const auto hue = sanitiseDegreesInt(static_cast<int>(std::floor(hct.hue())));
        huePopulation[hue] += population;
        populationSum += population;
    }

    std::array<double, 360> hueExcitedProportions{};
    for (int hue = 0; hue < 360; hue++) {
        const auto proportion = huePopulation[static_cast<std::size_t>(hue)] / populationSum;
        for (int i = hue - 14; i < hue + 16; i++)
            hueExcitedProportions[sanitiseDegreesInt(i)] += proportion;
    }

    std::vector<ScoredHct> scoredHct;
    scoredHct.reserve(coloursHct.size());
    for (const auto& hct : coloursHct) {
        const auto hue = sanitiseDegreesInt(static_cast<int>(std::round(hct.hue())));
        const auto proportion = hueExcitedProportions[hue];
        if (filter && (hct.chroma() < k_cutoffChroma || proportion <= k_cutoffExcitedProportion))
            continue;

        const auto proportionScore = proportion * 100.0 * k_weightProportion;
        const auto chromaWeight = hct.chroma() < k_targetChroma ? k_weightChromaBelow : k_weightChromaAbove;
        const auto chromaScore = (hct.chroma() - k_targetChroma) * chromaWeight;
        scoredHct.push_back({ .hct = hct, .score = proportionScore + chromaScore });
    }
    std::ranges::stable_sort(scoredHct, [](const ScoredHct& a, const ScoredHct& b) {
        return a.score > b.score;
    });
    return scoredHct;
}

std::optional<Hct> pickAboveCutoff(const std::vector<ScoredHct>& scoredHct) {
    for (int cutoff = 20; cutoff >= 0; cutoff--) {
        for (const auto& [hct, _] : scoredHct) {
            if (hct.chroma() > cutoff && hct.tone() > cutoff * 3)
                return hct;
        }
    }
    return std::nullopt;
}

} // namespace

std::vector<Argb> score(const std::map<Argb, std::uint32_t>& coloursToPopulation, const ScoreOptions& options) {
    const auto scoredHct = scoreColours(coloursToPopulation, options.filter);

    const auto desired = static_cast<std::size_t>(std::max(options.desired, 0));
    std::vector<const Hct*> chosenColours;
    chosenColours.reserve(desired);
    for (int differenceDeg = 90; differenceDeg >= 15; differenceDeg--) {
        chosenColours.clear();
        for (const auto& [hct, _] : scoredHct) {
            const auto duplicateHue = std::ranges::any_of(chosenColours, [&](const Hct* chosen) {
                return differenceDegrees(hct.hue(), chosen->hue()) < differenceDeg;
            });
            if (!duplicateHue)
                chosenColours.push_back(&hct);
            if (chosenColours.size() >= desired)
                break;
        }
        if (chosenColours.size() >= desired)
            break;
    }

    std::vector<Argb> colours;
    if (chosenColours.empty())
        colours.push_back(options.fallbackColour);
    colours.reserve(chosenColours.size());
    for (const auto* chosen : chosenColours)
        colours.push_back(chosen->toArgb());
    return colours;
}

Argb scoreSingle(const std::map<Argb, std::uint32_t>& coloursToPopulation, const ScoreOptions& options) {
    auto primary = pickAboveCutoff(scoreColours(coloursToPopulation, options.filter));
    if (!primary && options.filter)
        primary = pickAboveCutoff(scoreColours(coloursToPopulation, false));
    return primary ? primary->toArgb() : options.fallbackColour;
}

} // namespace m3colours::score
