#include "m3colours/score.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

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
    int cutoff = 0;
};

constexpr std::size_t sanitiseDegreesInt(int degrees) {
    degrees %= 360;
    return static_cast<std::size_t>(degrees < 0 ? degrees + 360 : degrees);
}

struct HueAnalysis {
    std::vector<Hct> coloursHct;
    std::array<double, 360> excitedProportions{};
};

HueAnalysis analyseHues(const std::map<Argb, std::uint32_t>& coloursToPopulation) {
    HueAnalysis analysis;
    analysis.coloursHct.reserve(coloursToPopulation.size());
    std::array<double, 360> huePopulation{};
    double populationSum = 0.0;
    for (const auto& [argb, population] : coloursToPopulation) {
        const auto& hct = analysis.coloursHct.emplace_back(argb);
        const auto hue = sanitiseDegreesInt(static_cast<int>(std::floor(hct.hue())));
        huePopulation[hue] += population;
        populationSum += population;
    }

    for (int hue = 0; hue < 360; hue++) {
        const auto proportion = huePopulation[static_cast<std::size_t>(hue)] / populationSum;
        for (int i = hue - 14; i < hue + 16; i++)
            analysis.excitedProportions[sanitiseDegreesInt(i)] += proportion;
    }
    return analysis;
}

double excitedProportion(const HueAnalysis& analysis, const Hct& hct) {
    return analysis.excitedProportions[sanitiseDegreesInt(static_cast<int>(std::round(hct.hue())))];
}

bool passesFilter(const Hct& hct, double proportion) {
    return hct.chroma() >= k_cutoffChroma && proportion > k_cutoffExcitedProportion;
}

double scoreColour(const Hct& hct, double proportion) {
    const auto proportionScore = proportion * 100.0 * k_weightProportion;
    const auto chromaWeight = hct.chroma() < k_targetChroma ? k_weightChromaBelow : k_weightChromaAbove;
    const auto chromaScore = (hct.chroma() - k_targetChroma) * chromaWeight;
    return proportionScore + chromaScore;
}

std::vector<ScoredHct> scoreColours(const std::map<Argb, std::uint32_t>& coloursToPopulation, bool filter) {
    const auto analysis = analyseHues(coloursToPopulation);

    std::vector<ScoredHct> scoredHct;
    scoredHct.reserve(analysis.coloursHct.size());
    for (const auto& hct : analysis.coloursHct) {
        const auto proportion = excitedProportion(analysis, hct);
        if (filter && !passesFilter(hct, proportion))
            continue;
        scoredHct.push_back({ .hct = hct, .score = scoreColour(hct, proportion) });
    }
    std::ranges::stable_sort(scoredHct, [](const ScoredHct& a, const ScoredHct& b) {
        return a.score > b.score;
    });
    return scoredHct;
}

// Largest cutoff in [0, 20] with chroma > cutoff and tone > 3 * cutoff, or -1
int maxCutoff(const Hct& hct) {
    const auto byChroma = static_cast<int>(std::ceil(hct.chroma())) - 1;
    auto byTone = static_cast<int>(std::ceil(hct.tone() / 3.0)) - 1;
    if (3.0 * (byTone + 1) < hct.tone())
        byTone++;
    else if (3.0 * byTone >= hct.tone())
        byTone--;
    return std::max(std::min({ 20, byChroma, byTone }), -1);
}

std::vector<ScoredHct> scoreColoursByCutoff(const HueAnalysis& analysis, bool filter) {
    std::vector<ScoredHct> scoredHct;
    for (const auto& hct : analysis.coloursHct) {
        const auto cutoff = maxCutoff(hct);
        if (cutoff < 0)
            continue;
        const auto proportion = excitedProportion(analysis, hct);
        if (filter && !passesFilter(hct, proportion))
            continue;
        scoredHct.push_back({ .hct = hct, .score = scoreColour(hct, proportion), .cutoff = cutoff });
    }
    std::ranges::stable_sort(scoredHct, [](const ScoredHct& a, const ScoredHct& b) {
        return a.cutoff != b.cutoff ? a.cutoff > b.cutoff : a.score > b.score;
    });
    return scoredHct;
}

std::vector<Argb> chooseDistinctHues(const std::vector<ScoredHct>& scoredHct, const ScoreOptions& options) {
    const auto desired = static_cast<std::size_t>(std::max(options.desired, 0));
    std::vector<const Hct*> chosenColours;
    chosenColours.reserve(desired);
    for (int differenceDeg = 90; differenceDeg >= 15; differenceDeg--) {
        chosenColours.clear();
        for (const auto& scored : scoredHct) {
            const auto duplicateHue = std::ranges::any_of(chosenColours, [&](const Hct* chosen) {
                return differenceDegrees(scored.hct.hue(), chosen->hue()) < differenceDeg;
            });
            if (!duplicateHue)
                chosenColours.push_back(&scored.hct);
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

} // namespace

std::vector<Argb> score(const std::map<Argb, std::uint32_t>& coloursToPopulation, const ScoreOptions& options) {
    return chooseDistinctHues(scoreColours(coloursToPopulation, options.filter), options);
}

std::vector<Argb> scoreByCutoff(const std::map<Argb, std::uint32_t>& coloursToPopulation, const ScoreOptions& options) {
    const auto analysis = analyseHues(coloursToPopulation);
    auto scoredHct = scoreColoursByCutoff(analysis, options.filter);
    if (scoredHct.empty() && options.filter)
        scoredHct = scoreColoursByCutoff(analysis, false);
    return chooseDistinctHues(scoredHct, options);
}

} // namespace m3colours::score
