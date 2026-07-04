#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3_add2.h>

#include "./ifc_data_schema.h"
#include "./ifc_data_utils.h"
#include "utils.h"

OpenBimRL::Engine::Types::IFC::pSet pSet;
OpenBimRL::Engine::Types::IFC::qSet qSet;

namespace OpenBimRL::Engine::Utils {
IFC::IfcData getData(IFC::IfcObjectPointer ifcPointer) {
    const auto entity = ifcPointer->as<IfcUtil::IfcBaseEntity>(true);
    return {.ifcClass = entity->declaration().name(),
            .GUID = entity->get_value<std::string>("GlobalId")};
}

void populateProperties(IFC::IfcData& data, IFC::IfcObjectPointer ifcObject) {
    pSet.clear();
    qSet.clear();

    if (isIFC4()) {
        populatePropertiesImpl<Ifc4>(ifcObject);
    } else if (isIFC4X3()) {
        populatePropertiesImpl<Ifc4x3_add2>(ifcObject);
    } else if (isIFC2x3()) {
        // IFC2x3 property extraction not implemented yet.
    }

    data.propertySets = pSet;
    data.quantitySets = qSet;
}
}  // namespace OpenBimRL::Engine::Utils
