# m3colours

An implementation of [Google's Material Colour Utilities][colour-utils] in C++.

The [C++ implementation][cpp-tree] in Google's official repo is outdated and
doesn't seem to be maintained, so I made this. This repo is made in reference
to the [official TypeScript implementation][ts-tree].

> [!NOTE]
> This implementation is not 1:1 with the official implementation. It uses some
> optimisations that change the results slightly, but also improve quantisation
> speed immensely (up to 13x or more, depending on the size of the image).

## Installation

This repo should be fetched using CMake's `FetchContent`. e.g.

```cmake
# Fetch lib
include(FetchContent)
FetchContent_Declare(m3colours
    GIT_REPOSITORY https://github.com/soramanew/m3colours.git
    GIT_TAG        main  # or pin a tag/commit
)
FetchContent_MakeAvailable(m3colours)

# Then link it
target_link_libraries(some_target PRIVATE m3colours::m3colours)
```

## Usage

The main usage is for image quantisation, colour scoring and scheme generation.

```c++
#include <cstdint>
#include <cstdio>
#include <vector>

#include <m3colours/quantise.hpp>
#include <m3colours/scheme.hpp>
#include <m3colours/score.hpp>

m3colours::scheme::DynamicScheme schemeForImage(unsigned char* data, std::size_t width, std::size_t height) {
    std::vector<m3colours::Argb> pixels;
    pixels.reserve(width * height);
    for (std::size_t i = 0; i < width * height; ++i) {
        // Assuming pixel data is in RGBA format (drop the alpha channel, we don't use it)
        const auto r = data[i * 4];
        const auto g = data[i * 4 + 1];
        const auto b = data[i * 4 + 2];
        pixels.emplace_back(r, g, b);
    }

    // Quantise the image, score the results and generate a scheme for the best colour
    const auto result = m3colours::quantise::celebi(pixels, 128);
    const auto ranked = m3colours::score::score(result.colourToCount); // Or scoreByCutoff
    const m3colours::scheme::SchemeTonalSpot scheme(m3colours::Hct{ ranked[0] }, true, 0);

    // Print the primary colour of the scheme
    std::printf("%06x\n", scheme.primary().value() & 0xffffffu);

    return scheme;
}
```

[colour-utils]: https://github.com/material-foundation/material-color-utilities
[cpp-tree]: https://github.com/material-foundation/material-color-utilities/tree/main/cpp
[ts-tree]: https://github.com/material-foundation/material-color-utilities/tree/main/typescript
