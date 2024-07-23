#include "utils.h"

std::string OpenBimRL::Engine::Utils::getGUID(Types::IFC::IfcObjectPointer ifcPointer) {
    if (isIFC2x3()) {
        const auto ifcItem = ifcPointer->as<Ifc2x3::IfcObject>();
        return ifcItem->GlobalId();
    }
    if (isIFC4()) {
        const auto ifcItem = ifcPointer->as<Ifc4::IfcObject>();
        return ifcItem->GlobalId();
    }

    throw std::runtime_error("not defined for other than IFC 2x3 or 4");
}