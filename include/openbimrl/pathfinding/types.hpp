#ifndef OPENBIMRL_NATIVE_PATHFINDING_TYPES_H
#define OPENBIMRL_NATIVE_PATHFINDING_TYPES_H

#include <cstdint>

namespace OpenBimRL::Native::Pathfinding {

struct Point2 {
    double x{};
    double y{};
};

struct Segment2 {
    double x1{};
    double y1{};
    double x2{};
    double y2{};
};

struct Edge {
    uint32_t a{};
    uint32_t b{};
};

}  // namespace OpenBimRL::Native::Pathfinding

#endif  // OPENBIMRL_NATIVE_PATHFINDING_TYPES_H
