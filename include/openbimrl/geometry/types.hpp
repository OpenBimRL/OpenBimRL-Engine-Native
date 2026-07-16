#ifndef OPENBIMRL_NATIVE_GEOMETRY_TYPES_H
#define OPENBIMRL_NATIVE_GEOMETRY_TYPES_H

#include "openbimrl/model/entity.hpp"

namespace OpenBimRL::Native::Geometry {

enum class FrameSource : int {
    NONE = 0,
    PLACEMENT = 1,
    REPRESENTATION = 2,
    BBOX = 3,
};

struct ElementFrame {
    double point[3]{};
    double axisX[3]{1.0, 0.0, 0.0};
    double axisZ[3]{0.0, 0.0, 1.0};
    FrameSource source{FrameSource::NONE};
};

struct Bounds3d {
    double minX{};
    double minY{};
    double minZ{};
    double maxX{};
    double maxY{};
    double maxZ{};
};

bool extractFrame(Model::IfcEntityRef entity, ElementFrame& out);

}  // namespace OpenBimRL::Native::Geometry

#endif  // OPENBIMRL_NATIVE_GEOMETRY_TYPES_H
