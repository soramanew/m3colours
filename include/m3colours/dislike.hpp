#pragma once

#include "hct.hpp"

namespace m3colours::dislike {

bool isDisliked(const Hct& hct);
Hct fixIfDisliked(const Hct& hct);

} // namespace m3colours::dislike
