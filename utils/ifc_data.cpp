#include "utils.h"

#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc2x3.h>

static OpenBimRL::Engine::Types::IFC::pSet pSet;
static OpenBimRL::Engine::Types::IFC::qSet qSet;

static std::string getPropValue(const Ifc4::IfcProperty *prop) {
    std::stringstream ss;

    if (const auto item = prop->as<Ifc4::IfcLabel>()) {
        ss << item->operator std::string();
    } else if (const auto item = prop->as<Ifc4::IfcIdentifier>()) {
        ss << item->operator std::string();
    } else if (const auto item = prop->as<Ifc4::IfcText>()) {
        ss << item->operator std::string();
    } else if (const auto item = prop->as<Ifc4::IfcBoolean>()) {
        ss << (item->operator bool() ? "true" : "false");
    } else if (const auto item = prop->as<Ifc4::IfcInteger>()) {
        ss << item->operator int();
    } else if (const auto item = prop->as<Ifc4::IfcReal>()) {
        ss << item->operator double();
    }

    return ss.str();
}

static double getQuantityValue(const Ifc4::IfcPhysicalQuantity *quantity) {
    if (const auto item = quantity->as<Ifc4::IfcQuantityArea>()) {
        return item->AreaValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityCount>()) {
        return item->CountValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityLength>()) {
        return item->LengthValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityTime>()) {
        return item->TimeValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityVolume>()) {
        return item->VolumeValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityWeight>()) {
        return item->WeightValue();
    }
    return -1.0;
}

static void handlePSet(const Ifc4::IfcPropertySet *set) {
    const auto name = set->Name();
    if (name->empty()) return;

    const auto props = set->HasProperties();

    for (const auto prop: (*props)) {
        pSet[name.value()][prop->Name()] = getPropValue(prop);
    }
}

static void handleQSet(const Ifc4::IfcQuantitySet *set) {
    const auto name = set->Name();
    if (name->empty()) return;

    const auto eq = set->as<Ifc4::IfcElementQuantity>();
    const auto quantities = eq->Quantities();
    for (const auto quantity: (*quantities)) {
        qSet[name.value()][quantity->Name()] = getQuantityValue(quantity);
    }
}

static void handleSetDefSelect(const Ifc4::IfcPropertySetDefinitionSelect *select) {
    if (const auto set = select->as<Ifc4::IfcPropertySet>()) {
        handlePSet(set);
        return;
    }

    if (const auto set = select->as<Ifc4::IfcQuantitySet>()) {
        handleQSet(set);
        return;
    }

    std::cout << "this script doesn't handle " << select->data().type()->name() << std::endl;
}

namespace OpenBimRL::Engine::Utils {
    IFC::IfcData getData(IFC::IfcObjectPointer ifcPointer) {
        if (isIFC2x3()) {
            const auto ifcItem = ifcPointer->as<Ifc2x3::IfcObject>();
            return {
                    .ifcClass = ifcItem->data().type()->name(),
                    .GUID = ifcItem->GlobalId()
            };
        }
        if (isIFC4()) {
            const auto ifcItem = ifcPointer->as<Ifc4::IfcObject>();
            return {
                    .ifcClass = ifcItem->data().type()->name(),
                    .GUID = ifcItem->GlobalId()
            };
        }

        throw std::runtime_error("not defined for other than IFC 2x3 or 4");
    }

    void populateProperties(IFC::IfcData &data, IFC::IfcObjectPointer ifcObject) {
        pSet.clear();
        qSet.clear();

        if (isIFC4()) {
            const auto product = ifcObject->as<Ifc4::IfcObject>();
            const auto definitions = product->IsDefinedBy();
            for (const auto definition: (*definitions)) {
                if (const auto byProps = definition->as<Ifc4::IfcRelDefinesByProperties>()) {
                    const auto propDefs = byProps->RelatingPropertyDefinition();

                    handleSetDefSelect(propDefs);
                    continue;
                }
                if (const auto byType = definition->as<Ifc4::IfcRelDefinesByType>()) {
                    const auto relatingType = byType->RelatingType();
                    const auto propertySetsOptional = relatingType->HasPropertySets();
                    if (!propertySetsOptional.has_value()) continue;

                    for (const auto propertySet: (*propertySetsOptional.value())) {
                        handleSetDefSelect(propertySet);
                    }
                    continue;
                }
            }
        } else if (isIFC2x3()) {
            const auto product = ifcObject->as<Ifc2x3::IfcObject>();
        }

        data.propertySets = pSet;
        data.quantitySets = qSet;
    }
}