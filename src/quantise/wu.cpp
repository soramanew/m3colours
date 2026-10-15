#include <array>
#include <optional>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

namespace {

enum class Direction : std::uint8_t {
    Red,
    Green,
    Blue,
};

constexpr std::size_t k_indexBits = 5;
constexpr std::size_t k_bitsToRemove = 8 - k_indexBits;
constexpr std::size_t k_indexCount = (1u << k_indexBits) + 1;
constexpr std::size_t k_totalSize = k_indexCount * k_indexCount * k_indexCount;
constexpr std::size_t k_maxColors = 256;

struct Box {
    std::size_t r0 = 0;
    std::size_t r1 = 0;
    std::size_t g0 = 0;
    std::size_t g1 = 0;
    std::size_t b0 = 0;
    std::size_t b1 = 0;

    [[nodiscard]] std::size_t volume() const { return (r1 - r0) * (g1 - g0) * (b1 - b0); }

    Box split(Direction direction, std::size_t position) {
        Box upper = *this;
        switch (direction) {
        case Direction::Red:
            upper.r0 = r1 = position;
            break;
        case Direction::Green:
            upper.g0 = g1 = position;
            break;
        case Direction::Blue:
            upper.b0 = b1 = position;
            break;
        }
        return upper;
    }
};

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

double squaredNorm(const Cell& cell) {
    const auto r = static_cast<double>(cell.r);
    const auto g = static_cast<double>(cell.g);
    const auto b = static_cast<double>(cell.b);
    return r * r + g * g + b * b;
}

std::size_t getIndex(std::size_t r, std::size_t g, std::size_t b) {
    return (r * k_indexCount + g) * k_indexCount + b;
}

class Moments {
public:
    void addPixel(Argb pixel) {
        const int64_t red = pixel.red();
        const int64_t green = pixel.green();
        const int64_t blue = pixel.blue();

        const auto indexR = (static_cast<std::size_t>(pixel.red()) >> k_bitsToRemove) + 1;
        const auto indexG = (static_cast<std::size_t>(pixel.green()) >> k_bitsToRemove) + 1;
        const auto indexB = (static_cast<std::size_t>(pixel.blue()) >> k_bitsToRemove) + 1;

        at(indexR, indexG, indexB) += Cell{
            .w = 1,
            .r = red,
            .g = green,
            .b = blue,
            .m2 = static_cast<double>(red * red + green * green + blue * blue),
        };
    }

    void accumulate() {
        for (std::size_t r = 1; r < k_indexCount; r++) {
            std::array<Cell, k_indexCount> area{};

            for (std::size_t g = 1; g < k_indexCount; g++) {
                Cell line;

                for (std::size_t b = 1; b < k_indexCount; b++) {
                    auto& column = area[b];
                    line += at(r, g, b);
                    column += line;
                    at(r, g, b) = at(r - 1, g, b) + column;
                }
            }
        }
    }

    [[nodiscard]] Cell sum(const Box& box) const {
        return at(box.r1, box.g1, box.b1) - at(box.r1, box.g1, box.b0) - at(box.r1, box.g0, box.b1) +
               at(box.r1, box.g0, box.b0) - at(box.r0, box.g1, box.b1) + at(box.r0, box.g1, box.b0) +
               at(box.r0, box.g0, box.b1) - at(box.r0, box.g0, box.b0);
    }

    [[nodiscard]] Cell top(const Box& box, Direction direction, std::size_t position) const {
        if (direction == Direction::Red)
            return at(position, box.g1, box.b1) - at(position, box.g1, box.b0) - at(position, box.g0, box.b1) +
                   at(position, box.g0, box.b0);
        if (direction == Direction::Green)
            return at(box.r1, position, box.b1) - at(box.r1, position, box.b0) - at(box.r0, position, box.b1) +
                   at(box.r0, position, box.b0);
        return at(box.r1, box.g1, position) - at(box.r1, box.g0, position) - at(box.r0, box.g1, position) +
               at(box.r0, box.g0, position);
    }

    [[nodiscard]] Cell bottom(const Box& box, Direction direction) const {
        if (direction == Direction::Red)
            return at(box.r0, box.g1, box.b0) - at(box.r0, box.g1, box.b1) + at(box.r0, box.g0, box.b1) -
                   at(box.r0, box.g0, box.b0);
        if (direction == Direction::Green)
            return at(box.r1, box.g0, box.b0) - at(box.r1, box.g0, box.b1) + at(box.r0, box.g0, box.b1) -
                   at(box.r0, box.g0, box.b0);
        return at(box.r1, box.g0, box.b0) - at(box.r1, box.g1, box.b0) + at(box.r0, box.g1, box.b0) -
               at(box.r0, box.g0, box.b0);
    }

    [[nodiscard]] double variance(const Box& box) const {
        const auto whole = sum(box);
        return whole.m2 - squaredNorm(whole) / static_cast<double>(whole.w);
    }

private:
    Cell& at(std::size_t r, std::size_t g, std::size_t b) { return m_cells[getIndex(r, g, b)]; }

    [[nodiscard]] const Cell& at(std::size_t r, std::size_t g, std::size_t b) const {
        return m_cells[getIndex(r, g, b)];
    }

    std::vector<Cell> m_cells = std::vector<Cell>(k_totalSize);
};

struct BestCut {
    double score = 0.0;
    std::optional<std::size_t> position;
};

BestCut maximise(const Moments& moments, const Box& box, Direction direction, std::size_t first, std::size_t last,
    const Cell& whole) {
    const auto base = moments.bottom(box, direction);

    BestCut best;
    for (std::size_t i = first; i < last; i++) {
        auto half = base + moments.top(box, direction, i);
        if (half.w == 0)
            continue;

        auto score = squaredNorm(half) / static_cast<double>(half.w);

        half = whole - half;
        if (half.w == 0)
            continue;

        score += squaredNorm(half) / static_cast<double>(half.w);

        if (score > best.score)
            best = { .score = score, .position = i };
    }
    return best;
}

std::optional<Box> cut(const Moments& moments, Box& box) {
    const auto whole = moments.sum(box);

    const auto red = maximise(moments, box, Direction::Red, box.r0 + 1, box.r1, whole);
    const auto green = maximise(moments, box, Direction::Green, box.g0 + 1, box.g1, whole);
    const auto blue = maximise(moments, box, Direction::Blue, box.b0 + 1, box.b1, whole);

    auto direction = Direction::Red;
    auto best = red;
    if (green.score > best.score) {
        direction = Direction::Green;
        best = green;
    }
    if (blue.score > best.score) {
        direction = Direction::Blue;
        best = blue;
    }

    if (!best.position)
        return std::nullopt;
    return box.split(direction, *best.position);
}

} // namespace

std::vector<Argb> wu(const std::vector<Argb>& pixels, uint16_t maxColours) {
    if (maxColours == 0 || maxColours > k_maxColors || pixels.empty())
        return {};

    std::size_t colourCount = maxColours;
    Moments moments;
    for (const auto pixel : pixels)
        moments.addPixel(pixel);
    moments.accumulate();

    std::vector<Box> cubes(k_maxColors);
    cubes[0].r1 = cubes[0].g1 = cubes[0].b1 = k_indexCount - 1;

    std::vector<double> volumeVariance(k_maxColors);
    std::size_t next = 0;
    for (std::size_t i = 1; i < colourCount; ++i) {
        if (const auto upper = cut(moments, cubes[next])) {
            cubes[i] = *upper;
            volumeVariance[next] = cubes[next].volume() > 1 ? moments.variance(cubes[next]) : 0.0;
            volumeVariance[i] = cubes[i].volume() > 1 ? moments.variance(cubes[i]) : 0.0;
        } else {
            volumeVariance[next] = 0.0;
            i--;
        }

        next = 0;
        double temp = volumeVariance[0];
        for (std::size_t j = 1; j <= i; j++) {
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
    for (std::size_t i = 0; i < colourCount; ++i) {
        const auto cell = moments.sum(cubes[i]);
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
