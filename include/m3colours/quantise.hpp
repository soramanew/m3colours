#pragma once

#include <vector>

#include "common.hpp"

namespace m3colours::quantise {

std::vector<Argb> wu(const std::vector<Argb>& pixels, uint16_t maxColours);

}
