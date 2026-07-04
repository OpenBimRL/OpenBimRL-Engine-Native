#include "utils.h"

std::string OpenBimRL::Engine::Utils::getGUID(Types::IFC::IfcObjectPointer ifcPointer) {
    const auto entity = ifcPointer->as<IfcUtil::IfcBaseEntity>(true);
    return entity->get_value<std::string>("GlobalId");
}
