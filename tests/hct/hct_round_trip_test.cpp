#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>

#include "m3colours/hct.hpp"

namespace m3colours::hct {

namespace {

TEST(HctRoundTrip, PreservesOriginalColour) {
    for (int r = 0; r < 296; r += 37) {
        for (int g = 0; g < 296; g += 37) {
            for (int b = 0; b < 296; b += 37) {
                const Argb argb(static_cast<std::uint8_t>(std::min(255, r)),
                    static_cast<std::uint8_t>(std::min(255, g)), static_cast<std::uint8_t>(std::min(255, b)));

                const Hct hct(argb);
                const Hct reconstructed(hct.hue(), hct.chroma(), hct.tone());
                EXPECT_EQ(reconstructed.toArgb(), argb);
            }
        }
    }
}

} // namespace

} // namespace m3colours::hct
