#include <algorithm>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

QuantiserResult celebi(const std::vector<Argb>& pixels, uint16_t maxColours, bool filterTransparent) {
    if (maxColours == 0 || pixels.empty())
        return {};

    maxColours = std::min<uint16_t>(maxColours, 256);

    if (!filterTransparent) {
        const auto wuResult = wu(pixels, maxColours);
        return wsMeans(pixels, wuResult, maxColours);
    }

    std::vector<Argb> opaque;
    opaque.reserve(pixels.size());
    std::ranges::copy_if(pixels, std::back_inserter(opaque), [](const Argb pixel) {
        return pixel.isOpaque();
    });

    const auto wuResult = wu(opaque, maxColours);
    return wsMeans(opaque, wuResult, maxColours);
}

} // namespace m3colours::quantise
