#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "m3colours/quantise.hpp"

namespace m3colours::quantise {

namespace {

constexpr int k_maxIterations = 100;
constexpr double k_minDeltaE = 3.0;
constexpr std::size_t k_maxColours = 256;
constexpr unsigned int k_seed = 42688;

double randomUnit() {
    return std::rand() / static_cast<double>(RAND_MAX); // NOLINT(misc-predictable-rand)
}

void reseed() {
    std::srand(k_seed); // NOLINT(bugprone-random-generator-seed)
}

struct Centroid {
    double l = 0.0;
    double a = 0.0;
    double b = 0.0;
    std::uint32_t count = 0;
};

class WsMeans {
public:
    WsMeans(const WeightedPixels& inputPixels, const std::vector<Argb>& startingClusters, std::size_t maxColours);

    QuantiserResult run();

private:
    void initialiseClusters(const std::vector<Argb>& startingClusters, std::size_t clusterCount);
    void assignRandomClusters();
    void computeClusterDistances();
    bool reassignPoints();
    void recomputeCentres();
    [[nodiscard]] QuantiserResult buildResult() const;

    [[nodiscard]] double clusterDistance(std::size_t i, std::size_t j) const {
        return m_clusterDistances[(i * m_clusters.size()) + j];
    }

    const std::vector<Argb>& m_pixels;
    const std::vector<std::uint32_t>& m_counts;
    std::vector<Lab> m_points;
    std::vector<Lab> m_clusters;
    std::vector<std::size_t> m_clusterIndices;
    std::vector<double> m_clusterDistances;
    std::vector<std::uint32_t> m_populations;
};

WsMeans::WsMeans(const WeightedPixels& inputPixels, const std::vector<Argb>& startingClusters, std::size_t maxColours)
    : m_pixels(inputPixels.pixels)
    , m_counts(inputPixels.counts) {
    m_points.reserve(m_pixels.size());
    for (const auto pixel : m_pixels)
        m_points.emplace_back(pixel);

    auto clusterCount = std::min(maxColours, m_points.size());
    if (!startingClusters.empty())
        clusterCount = std::min(clusterCount, startingClusters.size());

    initialiseClusters(startingClusters, clusterCount);
    assignRandomClusters();
    m_clusterDistances.resize(clusterCount * clusterCount);
    m_populations.resize(clusterCount);
}

QuantiserResult WsMeans::run() {
    for (int iteration = 0; iteration < k_maxIterations; iteration++) {
        computeClusterDistances();
        if (!reassignPoints() && iteration != 0)
            break;
        recomputeCentres();
    }
    return buildResult();
}

void WsMeans::initialiseClusters(const std::vector<Argb>& startingClusters, std::size_t clusterCount) {
    m_clusters.reserve(clusterCount);
    if (!startingClusters.empty()) {
        for (std::size_t i = 0; i < clusterCount; i++)
            m_clusters.emplace_back(startingClusters[i]);
        return;
    }

    reseed();
    for (std::size_t i = 0; i < clusterCount; i++) {
        const auto l = randomUnit() * 100.0;
        const auto a = randomUnit() * 200.0 - 100.0;
        const auto b = randomUnit() * 200.0 - 100.0;
        m_clusters.emplace_back(l, a, b);
    }
}

void WsMeans::assignRandomClusters() {
    m_clusterIndices.reserve(m_points.size());
    reseed();
    for (std::size_t i = 0; i < m_points.size(); i++)
        m_clusterIndices.push_back(
            static_cast<std::size_t>(std::rand()) % m_clusters.size()); // NOLINT(misc-predictable-rand)
}

void WsMeans::computeClusterDistances() {
    const auto clusterCount = m_clusters.size();
    for (std::size_t i = 0; i < clusterCount; i++) {
        m_clusterDistances[(i * clusterCount) + i] = 0.0;
        for (std::size_t j = i + 1; j < clusterCount; j++) {
            const auto distance = m_clusters[i].deltaE(m_clusters[j]);
            m_clusterDistances[(i * clusterCount) + j] = distance;
            m_clusterDistances[(j * clusterCount) + i] = distance;
        }
    }
}

bool WsMeans::reassignPoints() {
    bool colourMoved = false;
    for (std::size_t i = 0; i < m_points.size(); i++) {
        const auto& point = m_points[i];
        const auto previous = m_clusterIndices[i];
        const auto previousDistance = point.deltaE(m_clusters[previous]);
        auto minimumDistance = previousDistance;
        auto nearest = previous;

        for (std::size_t j = 0; j < m_clusters.size(); j++) {
            if (clusterDistance(previous, j) >= 4 * previousDistance)
                continue;

            const auto distance = point.deltaE(m_clusters[j]);
            if (distance < minimumDistance) {
                minimumDistance = distance;
                nearest = j;
            }
        }

        if (nearest == previous)
            continue;

        const auto distanceChange = std::abs(std::sqrt(minimumDistance) - std::sqrt(previousDistance));
        if (distanceChange > k_minDeltaE) {
            colourMoved = true;
            m_clusterIndices[i] = nearest;
        }
    }
    return colourMoved;
}

void WsMeans::recomputeCentres() {
    std::vector<Centroid> centroids(m_clusters.size());
    for (std::size_t i = 0; i < m_points.size(); i++) {
        auto& centroid = centroids[m_clusterIndices[i]];
        const auto& point = m_points[i];
        const auto count = m_counts[i];

        centroid.count += count;
        centroid.l += point.l * count;
        centroid.a += point.a * count;
        centroid.b += point.b * count;
    }

    for (std::size_t i = 0; i < m_clusters.size(); i++) {
        const auto& centroid = centroids[i];
        m_populations[i] = centroid.count;
        if (centroid.count == 0) {
            m_clusters[i] = Lab{ 0, 0, 0 };
            continue;
        }
        m_clusters[i] = { centroid.l / centroid.count, centroid.a / centroid.count, centroid.b / centroid.count };
    }
}

QuantiserResult WsMeans::buildResult() const {
    QuantiserResult result;

    std::vector<Argb> clusterArgbs;
    clusterArgbs.reserve(m_clusters.size());
    for (std::size_t i = 0; i < m_clusters.size(); i++) {
        const auto argb = m_clusters[i].toArgb();
        clusterArgbs.push_back(argb);
        if (m_populations[i] != 0)
            result.colourToCount[argb] += m_populations[i];
    }

    result.inputToCluster.reserve(m_pixels.size());
    for (std::size_t i = 0; i < m_pixels.size(); i++)
        result.inputToCluster.emplace_back(m_pixels[i], clusterArgbs[m_clusterIndices[i]]);
    std::ranges::sort(result.inputToCluster, {}, &std::pair<Argb, Argb>::first);

    return result;
}

} // namespace

QuantiserResult wsMeans(
    const std::vector<Argb>& inputPixels, const std::vector<Argb>& startingClusters, uint16_t maxColours) {
    return internal::wsMeans(internal::uniquePixels(inputPixels, false), startingClusters, maxColours);
}

QuantiserResult internal::wsMeans(
    const WeightedPixels& inputPixels, const std::vector<Argb>& startingClusters, uint16_t maxColours) {
    if (maxColours == 0 || inputPixels.pixels.empty())
        return {};

    return WsMeans(inputPixels, startingClusters, std::min<std::size_t>(maxColours, k_maxColours)).run();
}

} // namespace m3colours::quantise
