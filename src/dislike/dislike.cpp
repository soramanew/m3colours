#include "m3colours/dislike.hpp"

#include <cmath>

namespace m3colours::dislike {

bool isDisliked(const Hct& hct) {
    const auto hue = std::round(hct.hue());
    const auto huePasses = hue >= 90.0 && hue <= 111.0;
    const auto chromaPasses = std::round(hct.chroma()) > 16.0;
    const auto tonePasses = std::round(hct.tone()) < 65.0;
    return huePasses && chromaPasses && tonePasses;
}

Hct fixIfDisliked(const Hct& hct) {
    if (isDisliked(hct))
        return { hct.hue(), hct.chroma(), 70.0 };
    return hct;
}

} // namespace m3colours::dislike
