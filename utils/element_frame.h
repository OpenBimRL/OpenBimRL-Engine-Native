#ifndef OPENBIMRL_NATIVE_ELEMENT_FRAME_H
#define OPENBIMRL_NATIVE_ELEMENT_FRAME_H

#include "types.h"

namespace OpenBimRL::Engine::Utils {

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

bool getElementFrame(Types::IFC::IfcObjectPointer entity, ElementFrame& out);

}  // namespace OpenBimRL::Engine::Utils

#endif  // OPENBIMRL_NATIVE_ELEMENT_FRAME_H
