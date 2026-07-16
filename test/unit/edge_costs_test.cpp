#include <cmath>
#include <gtest/gtest.h>

#include "openbimrl/pathfinding/edge_costs.hpp"

using OpenBimRL::Native::Pathfinding::computeEdgeCosts;
using OpenBimRL::Native::Pathfinding::Edge;
using OpenBimRL::Native::Pathfinding::Point2;
using OpenBimRL::Native::Pathfinding::Segment2;
using OpenBimRL::Native::Pathfinding::segmentsIntersect;

TEST(Pathfinding, SegmentsIntersectCrossing) {
    const Segment2 a{0.0, 0.0, 1.0, 1.0};
    const Segment2 b{0.0, 1.0, 1.0, 0.0};
    EXPECT_TRUE(segmentsIntersect(a, b));
}

TEST(Pathfinding, SegmentsDoNotIntersectParallel) {
    const Segment2 a{0.0, 0.0, 1.0, 0.0};
    const Segment2 b{0.0, 1.0, 1.0, 1.0};
    EXPECT_FALSE(segmentsIntersect(a, b));
}

TEST(Pathfinding, EdgeCostObstacleBlocks) {
    const std::vector<Point2> points{{0.0, 0.0}, {2.0, 0.0}};
    const std::vector<Edge> edges{{0, 1}};
    const std::vector<Segment2> passages;
    const std::vector<Segment2> obstacles{{1.0, -1.0, 1.0, 1.0}};
    std::vector<double> costs(1);
    computeEdgeCosts(points, edges, passages, obstacles, costs);
    EXPECT_TRUE(std::isinf(costs[0]));
}

TEST(Pathfinding, EdgeCostPassageOverridesObstacle) {
    const std::vector<Point2> points{{0.0, 0.0}, {2.0, 0.0}};
    const std::vector<Edge> edges{{0, 1}};
    const std::vector<Segment2> passages{{1.0, -1.0, 1.0, 1.0}};
    const std::vector<Segment2> obstacles{{1.0, -1.0, 1.0, 1.0}};
    std::vector<double> costs(1);
    computeEdgeCosts(points, edges, passages, obstacles, costs);
    EXPECT_NEAR(costs[0], 2.0, 1e-9);
}

TEST(Pathfinding, EdgeCostClearIsEuclidean) {
    const std::vector<Point2> points{{0.0, 0.0}, {3.0, 4.0}};
    const std::vector<Edge> edges{{0, 1}};
    std::vector<double> costs(1);
    computeEdgeCosts(points, edges, {}, {}, costs);
    EXPECT_NEAR(costs[0], 5.0, 1e-9);
}
