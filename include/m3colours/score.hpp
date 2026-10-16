#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "common.hpp"

namespace m3colours::score {

struct ScoreOptions {
    int desired = 4;
    Argb fallbackColour{ 0xff4285f4 };
    bool filter = true;
};

std::vector<Argb> score(const std::map<Argb, std::uint32_t>& coloursToPopulation, const ScoreOptions& options = {});
std::vector<Argb> scoreByCutoff(
    const std::map<Argb, std::uint32_t>& coloursToPopulation, const ScoreOptions& options = {});

} // namespace m3colours::score
