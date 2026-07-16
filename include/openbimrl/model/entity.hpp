#ifndef OPENBIMRL_NATIVE_MODEL_ENTITY_H
#define OPENBIMRL_NATIVE_MODEL_ENTITY_H

namespace OpenBimRL::Native::Model {

/** Opaque IFC entity handle (IfcUtil::IfcBaseClass*). Safe to pass across JNA. */
struct IfcEntityRef {
    void* ptr{nullptr};

    explicit operator bool() const { return ptr != nullptr; }
};

}  // namespace OpenBimRL::Native::Model

#endif  // OPENBIMRL_NATIVE_MODEL_ENTITY_H
