#pragma once

#include "hct.hpp"

namespace m3colours::dislike {

bool isDisliked(const hct::Hct& hct);
hct::Hct fixIfDisliked(const hct::Hct& hct);

} // namespace m3colours::dislike
