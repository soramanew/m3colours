#include <array>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

namespace {

struct Box {
    int r0 = 0;
    int r1 = 0;
    int g0 = 0;
    int g1 = 0;
    int b0 = 0;
    int b1 = 0;
    int vol = 0;
};

enum class Direction : std::uint8_t {
    Red,
    Green,
    Blue,
};

constexpr int k_indexBits = 5;
constexpr int k_indexCount = ((1 << k_indexBits) + 1);
constexpr int k_totalSize = (k_indexCount * k_indexCount * k_indexCount);
constexpr int k_maxColors = 256;

struct Cell {
    int64_t w = 0;
    int64_t r = 0;
    int64_t g = 0;
    int64_t b = 0;
    double m2 = 0.0;

    Cell& operator+=(const Cell& o) {
        w += o.w;
        r += o.r;
        g += o.g;
        b += o.b;
        m2 += o.m2;
        return *this;
    }

    Cell& operator-=(const Cell& o) {
        w -= o.w;
        r -= o.r;
        g -= o.g;
        b -= o.b;
        m2 -= o.m2;
        return *this;
    }

    friend Cell operator+(Cell lhs, const Cell& rhs) { return lhs += rhs; }

    friend Cell operator-(Cell lhs, const Cell& rhs) { return lhs -= rhs; }
};

using Moments = std::vector<Cell>;

size_t getIndex(int r, int g, int b) {
    const auto index = (r * k_indexCount + g) * k_indexCount + b;
    return static_cast<size_t>(index);
}

void constructHistogram(const std::vector<Argb>& pixels, Moments& moments) {
    for (const auto pixel : pixels) {
        const int64_t red = pixel.red();
        const int64_t green = pixel.green();
        const int64_t blue = pixel.blue();

        const auto bitsToRemove = 8 - k_indexBits;
        const auto indexR = (pixel.red() >> bitsToRemove) + 1;
        const auto indexG = (pixel.green() >> bitsToRemove) + 1;
        const auto indexB = (pixel.blue() >> bitsToRemove) + 1;
        const auto index = getIndex(indexR, indexG, indexB);

        moments[index] += Cell{
            .w = 1,
            .r = red,
            .g = green,
            .b = blue,
            .m2 = static_cast<double>(red * red + green * green + blue * blue),
        };
    }
}

void computeMoments(Moments& moments) {
    for (int r = 1; r < k_indexCount; r++) {
        std::array<Cell, k_indexCount> area{};

        for (int g = 1; g < k_indexCount; g++) {
            Cell line;

            for (int b = 1; b < k_indexCount; b++) {
                const auto index = getIndex(r, g, b);
                auto& column = area[static_cast<size_t>(b)];
                line += moments[index];
                column += line;
                moments[index] = moments[getIndex(r - 1, g, b)] + column;
            }
        }
    }
}

Cell top(const Box& cube, const Direction direction, const int position, const Moments& moments) {
    if (direction == Direction::Red)
        return moments[getIndex(position, cube.g1, cube.b1)] - moments[getIndex(position, cube.g1, cube.b0)] -
               moments[getIndex(position, cube.g0, cube.b1)] + moments[getIndex(position, cube.g0, cube.b0)];
    if (direction == Direction::Green)
        return moments[getIndex(cube.r1, position, cube.b1)] - moments[getIndex(cube.r1, position, cube.b0)] -
               moments[getIndex(cube.r0, position, cube.b1)] + moments[getIndex(cube.r0, position, cube.b0)];
    return moments[getIndex(cube.r1, cube.g1, position)] - moments[getIndex(cube.r1, cube.g0, position)] -
           moments[getIndex(cube.r0, cube.g1, position)] + moments[getIndex(cube.r0, cube.g0, position)];
}

Cell bottom(const Box& cube, const Direction direction, const Moments& moments) {
    if (direction == Direction::Red)
        return moments[getIndex(cube.r0, cube.g1, cube.b0)] - moments[getIndex(cube.r0, cube.g1, cube.b1)] +
               moments[getIndex(cube.r0, cube.g0, cube.b1)] - moments[getIndex(cube.r0, cube.g0, cube.b0)];
    if (direction == Direction::Green)
        return moments[getIndex(cube.r1, cube.g0, cube.b0)] - moments[getIndex(cube.r1, cube.g0, cube.b1)] +
               moments[getIndex(cube.r0, cube.g0, cube.b1)] - moments[getIndex(cube.r0, cube.g0, cube.b0)];
    return moments[getIndex(cube.r1, cube.g0, cube.b0)] - moments[getIndex(cube.r1, cube.g1, cube.b0)] +
           moments[getIndex(cube.r0, cube.g1, cube.b0)] - moments[getIndex(cube.r0, cube.g0, cube.b0)];
}

Cell vol(const Box& cube, const Moments& moments) {
    return moments[getIndex(cube.r1, cube.g1, cube.b1)] - moments[getIndex(cube.r1, cube.g1, cube.b0)] -
           moments[getIndex(cube.r1, cube.g0, cube.b1)] + moments[getIndex(cube.r1, cube.g0, cube.b0)] -
           moments[getIndex(cube.r0, cube.g1, cube.b1)] + moments[getIndex(cube.r0, cube.g1, cube.b0)] +
           moments[getIndex(cube.r0, cube.g0, cube.b1)] - moments[getIndex(cube.r0, cube.g0, cube.b0)];
}

double squaredNorm(const Cell& cell) {
    const auto r = static_cast<double>(cell.r);
    const auto g = static_cast<double>(cell.g);
    const auto b = static_cast<double>(cell.b);
    return r * r + g * g + b * b;
}

double variance(const Box& cube, const Moments& moments) {
    const auto whole = vol(cube, moments);
    return whole.m2 - squaredNorm(whole) / static_cast<double>(whole.w);
}

double maximize(const Box& cube, const Direction direction, const int first, const int last, int* cut,
    const Cell& whole, const Moments& moments) {
    const auto base = bottom(cube, direction, moments);

    double max = 0.0;
    *cut = -1;

    for (int i = first; i < last; i++) {
        auto half = base + top(cube, direction, i, moments);
        if (half.w == 0) {
            continue;
        }

        double temp = squaredNorm(half) / static_cast<double>(half.w);

        half = whole - half;
        if (half.w == 0) {
            continue;
        }
        temp += squaredNorm(half) / static_cast<double>(half.w);

        if (temp > max) {
            max = temp;
            *cut = i;
        }
    }
    return max;
}

bool cut(Box& box1, Box& box2, const Moments& moments) {
    const auto whole = vol(box1, moments);

    int cutR;
    int cutG;
    int cutB;
    const auto maxR = maximize(box1, Direction::Red, box1.r0 + 1, box1.r1, &cutR, whole, moments);
    const auto maxG = maximize(box1, Direction::Green, box1.g0 + 1, box1.g1, &cutG, whole, moments);
    const auto maxB = maximize(box1, Direction::Blue, box1.b0 + 1, box1.b1, &cutB, whole, moments);

    Direction direction;
    if (maxR >= maxG && maxR >= maxB) {
        direction = Direction::Red;
        if (cutR < 0) {
            return false;
        }
    } else if (maxG >= maxR && maxG >= maxB) {
        direction = Direction::Green;
    } else {
        direction = Direction::Blue;
    }

    box2.r1 = box1.r1;
    box2.g1 = box1.g1;
    box2.b1 = box1.b1;

    if (direction == Direction::Red) {
        box2.r0 = box1.r1 = cutR;
        box2.g0 = box1.g0;
        box2.b0 = box1.b0;
    } else if (direction == Direction::Green) {
        box2.r0 = box1.r0;
        box2.g0 = box1.g1 = cutG;
        box2.b0 = box1.b0;
    } else {
        box2.r0 = box1.r0;
        box2.g0 = box1.g0;
        box2.b0 = box1.b1 = cutB;
    }

    box1.vol = (box1.r1 - box1.r0) * (box1.g1 - box1.g0) * (box1.b1 - box1.b0);
    box2.vol = (box2.r1 - box2.r0) * (box2.g1 - box2.g0) * (box2.b1 - box2.b0);
    return true;
}

} // namespace

std::vector<Argb> wu(const std::vector<Argb>& pixels, uint16_t maxColours) {
    if (maxColours <= 0 || maxColours > 256 || pixels.empty())
        return {};

    size_t colourCount = maxColours;
    Moments moments(k_totalSize);
    constructHistogram(pixels, moments);
    computeMoments(moments);

    std::vector<Box> cubes(k_maxColors);
    cubes[0].r0 = cubes[0].g0 = cubes[0].b0 = 0;
    cubes[0].r1 = cubes[0].g1 = cubes[0].b1 = k_indexCount - 1;

    std::vector<double> volumeVariance(k_maxColors);
    size_t next = 0;
    for (size_t i = 1; i < colourCount; ++i) {
        if (cut(cubes[next], cubes[i], moments)) {
            volumeVariance[next] = cubes[next].vol > 1 ? variance(cubes[next], moments) : 0.0;
            volumeVariance[i] = cubes[i].vol > 1 ? variance(cubes[i], moments) : 0.0;
        } else {
            volumeVariance[next] = 0.0;
            i--;
        }

        next = 0;
        double temp = volumeVariance[0];
        for (size_t j = 1; j <= i; j++) {
            if (volumeVariance[j] > temp) {
                temp = volumeVariance[j];
                next = j;
            }
        }
        if (temp <= 0.0) {
            colourCount = i + 1;
            break;
        }
    }

    std::vector<Argb> outColours;
    for (size_t i = 0; i < colourCount; ++i) {
        const auto cell = vol(cubes[i], moments);
        if (cell.w > 0) {
            const auto red = static_cast<uint8_t>(cell.r / cell.w);
            const auto green = static_cast<uint8_t>(cell.g / cell.w);
            const auto blue = static_cast<uint8_t>(cell.b / cell.w);
            outColours.emplace_back(red, green, blue);
        }
    }

    return outColours;
}

} // namespace m3colours::quantise
