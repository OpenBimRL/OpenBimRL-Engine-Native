#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>

#include "./ifc_data_utils.h"
#include "utils.h"

OpenBimRL::Engine::Types::IFC::pSet pSet;
OpenBimRL::Engine::Types::IFC::qSet qSet;

namespace OpenBimRL::Engine::Utils {
IFC::IfcData getData(IFC::IfcObjectPointer ifcPointer) {
    if (isIFC2x3()) {
        const auto ifcItem = ifcPointer->as<Ifc2x3::IfcObject>();
        return {.ifcClass = ifcItem->data().type()->name(),
                .GUID = ifcItem->GlobalId()};
    }
    if (isIFC4()) {
        const auto ifcItem = ifcPointer->as<Ifc4::IfcObject>();
        return {.ifcClass = ifcItem->data().type()->name(),
                .GUID = ifcItem->GlobalId()};
    }

    throw std::runtime_error("not defined for other than IFC 2x3 or 4");
}

void populateProperties(IFC::IfcData &data, IFC::IfcObjectPointer ifcObject) {
    pSet.clear();
    qSet.clear();

    if (isIFC4()) {
        const auto product = ifcObject->as<Ifc4::IfcObject>();
        const auto definitions = product->IsDefinedBy();
        for (const auto definition : (*definitions)) {
            if (const auto byProps =
                    definition->as<Ifc4::IfcRelDefinesByProperties>()) {
                const auto propDefs = byProps->RelatingPropertyDefinition();

                handleSetDefSelect(propDefs);
                continue;
            }
            if (const auto byType =
                    definition->as<Ifc4::IfcRelDefinesByType>()) {
                const auto relatingType = byType->RelatingType();
                const auto propertySetsOptional =
                    relatingType->HasPropertySets();
                if (!propertySetsOptional.has_value()) continue;

                for (const auto propertySet : (*propertySetsOptional.value())) {
                    handleSetDefSelect(propertySet);
                }
                continue;
            }
        }
    } else if (isIFC2x3()) {
        // const auto product = ifcObject->as<Ifc2x3::IfcObject>();
    }

    data.propertySets = pSet;
    data.quantitySets = qSet;
}
}  // namespace OpenBimRL::Engine::Utils