#pragma once

#include "common.hpp"

namespace m3colours::blend {

Argb harmonise(Argb designColour, Argb sourceColour, double maxRotation = 15.0);
Argb hctHue(Argb from, Argb to, double amount);
Argb cam16Ucs(Argb from, Argb to, double amount);

} // namespace m3colours::blend
