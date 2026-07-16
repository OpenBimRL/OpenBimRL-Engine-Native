#ifndef OPENBIMRL_NATIVE_PATHFINDING_EDGE_COSTS_H
#define OPENBIMRL_NATIVE_PATHFINDING_EDGE_COSTS_H

#include <cstddef>
#include <span>

#include "openbimrl/pathfinding/types.hpp"

namespace OpenBimRL::Native::Pathfinding {

/** True if segments a and b intersect (including endpoint-on-segment). */
bool segmentsIntersect(const Segment2& a, const Segment2& b);

/**
 * Compute per-edge traversal costs.
 * Passage hits → Euclidean length; obstacle hits (without passage) → +inf;
 * otherwise Euclidean length.
 */
void computeEdgeCosts(std::span<const Point2> points,
                      std::span<const Edge> edges,
                      std::span<const Segment2> passages,
                      std::span<const Segment2> obstacles,
                      std::span<double> outCosts);

}  // namespace OpenBimRL::Native::Pathfinding

#endif  // OPENBIMRL_NATIVE_PATHFINDING_EDGE_COSTS_H
