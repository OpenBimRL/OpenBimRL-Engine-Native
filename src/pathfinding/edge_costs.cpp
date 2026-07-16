#include "openbimrl/pathfinding/edge_costs.hpp"

#include <omp.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace OpenBimRL::Native::Pathfinding {
namespace {

constexpr double EPS = 1e-9;

double orientation(double ax, double ay, double bx, double by, double cx,
                   double cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

bool onSegment(double ax, double ay, double bx, double by, double px,
               double py) {
    return px >= std::min(ax, bx) - EPS && px <= std::max(ax, bx) + EPS &&
           py >= std::min(ay, by) - EPS && py <= std::max(ay, by) + EPS;
}

bool intersectsAny(const Segment2& line, std::span<const Segment2> segs) {
    for (const auto& seg : segs) {
        if (segmentsIntersect(line, seg)) return true;
    }
    return false;
}

}  // namespace

bool segmentsIntersect(const Segment2& a, const Segment2& b) {
    const auto o1 = orientation(a.x1, a.y1, a.x2, a.y2, b.x1, b.y1);
    const auto o2 = orientation(a.x1, a.y1, a.x2, a.y2, b.x2, b.y2);
    const auto o3 = orientation(b.x1, b.y1, b.x2, b.y2, a.x1, a.y1);
    const auto o4 = orientation(b.x1, b.y1, b.x2, b.y2, a.x2, a.y2);

    const auto sgn = [](double v) -> int {
        if (v > EPS) return 1;
        if (v < -EPS) return -1;
        return 0;
    };

    const auto s1 = sgn(o1);
    const auto s2 = sgn(o2);
    const auto s3 = sgn(o3);
    const auto s4 = sgn(o4);

    if (s1 != s2 && s3 != s4) return true;
    if (s1 == 0 && onSegment(a.x1, a.y1, a.x2, a.y2, b.x1, b.y1)) return true;
    if (s2 == 0 && onSegment(a.x1, a.y1, a.x2, a.y2, b.x2, b.y2)) return true;
    if (s3 == 0 && onSegment(b.x1, b.y1, b.x2, b.y2, a.x1, a.y1)) return true;
    if (s4 == 0 && onSegment(b.x1, b.y1, b.x2, b.y2, a.x2, a.y2)) return true;
    return false;
}

void computeEdgeCosts(std::span<const Point2> points,
                      std::span<const Edge> edges,
                      std::span<const Segment2> passages,
                      std::span<const Segment2> obstacles,
                      std::span<double> outCosts) {
    if (outCosts.size() < edges.size()) return;

    const auto edgeCount = static_cast<uint32_t>(edges.size());
    const auto pointCount = static_cast<uint32_t>(points.size());

#pragma omp parallel for if (edgeCount > 256)
    for (uint32_t i = 0; i < edgeCount; ++i) {
        const auto aIdx = edges[i].a;
        const auto bIdx = edges[i].b;
        if (aIdx >= pointCount || bIdx >= pointCount) {
            outCosts[i] = std::numeric_limits<double>::infinity();
            continue;
        }
        const auto ax = points[aIdx].x;
        const auto ay = points[aIdx].y;
        const auto bx = points[bIdx].x;
        const auto by = points[bIdx].y;
        const Segment2 line{ax, ay, bx, by};

        const auto dx = ax - bx;
        const auto dy = ay - by;
        const auto euclidean = std::sqrt(dx * dx + dy * dy);

        if (intersectsAny(line, passages)) {
            outCosts[i] = euclidean;
            continue;
        }
        if (intersectsAny(line, obstacles)) {
            outCosts[i] = std::numeric_limits<double>::infinity();
            continue;
        }
        outCosts[i] = euclidean;
    }
}

}  // namespace OpenBimRL::Native::Pathfinding
