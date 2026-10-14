#include <algorithm>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

QuantiserResult celebi(const std::vector<Argb>& pixels, uint16_t maxColours) {
    if (maxColours == 0 || pixels.empty())
        return {};

    maxColours = std::min<uint16_t>(maxColours, 256);

    std::vector<Argb> opaquePixels;
    opaquePixels.reserve(pixels.size());
    for (const auto pixel : pixels) {
        if (!pixel.isOpaque())
            continue;
        opaquePixels.push_back(pixel);
    }

    const auto wuResult = wu(opaquePixels, maxColours);
    return wsMeans(opaquePixels, wuResult, maxColours);
}

} // namespace m3colours::quantise
