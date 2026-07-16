#ifndef OPENBIMRL_NATIVE_COMPAT_H
#define OPENBIMRL_NATIVE_COMPAT_H

/**
 * Compatibility facade for existing integration tests.
 * New code should use OpenBimRL::Native::* directly.
 */

#include <ifcparse/IfcFile.h>

#include "openbimrl/geometry/types.hpp"
#include "openbimrl/model/session.hpp"
#include "openbimrl/properties/data.hpp"

namespace OpenBimRL::Engine::Utils {

inline void setSilent(bool s) {
    OpenBimRL::Native::Model::ActiveSession::setSilent(s);
}

inline IfcParse::IfcFile* getCurrentFile() {
    return OpenBimRL::Native::Model::ActiveSession::file();
}

inline bool isIFC2x3() {
    auto* s = OpenBimRL::Native::Model::ActiveSession::get();
    return s && s->isIFC2x3();
}

inline bool isIFC4() {
    auto* s = OpenBimRL::Native::Model::ActiveSession::get();
    return s && s->isIFC4();
}

inline bool isIFC4X3() {
    auto* s = OpenBimRL::Native::Model::ActiveSession::get();
    return s && s->isIFC4X3();
}

inline std::string getGUID(IfcUtil::IfcBaseClass* ptr) {
    return OpenBimRL::Native::Model::getGuid(
        OpenBimRL::Native::Model::IfcEntityRef{ptr});
}

inline OpenBimRL::Native::Properties::IfcData getData(
    IfcUtil::IfcBaseClass* ptr) {
    auto* session = OpenBimRL::Native::Model::ActiveSession::get();
    if (!session) return {};
    return OpenBimRL::Native::Properties::snapshot(
        *session, OpenBimRL::Native::Model::IfcEntityRef{ptr});
}

using ElementFrame = OpenBimRL::Native::Geometry::ElementFrame;
using FrameSource = OpenBimRL::Native::Geometry::FrameSource;

inline bool getElementFrame(IfcUtil::IfcBaseClass* ptr, ElementFrame& out) {
    return OpenBimRL::Native::Geometry::extractFrame(
        OpenBimRL::Native::Model::IfcEntityRef{ptr}, out);
}

}  // namespace OpenBimRL::Engine::Utils

#endif  // OPENBIMRL_NATIVE_COMPAT_H
