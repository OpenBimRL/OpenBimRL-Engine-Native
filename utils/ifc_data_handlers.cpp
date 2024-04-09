#include <iostream>
#include "./ifc_data_utils.h"

void handlePSet(const Ifc4::IfcPropertySet *set) {
    if (set->Name()->empty()) return;

    const auto setName = set->Name().value();

    const auto props = set->HasProperties();

    for (const auto prop: (*props)) {
        const auto propName = prop->Name();
        const auto propIfcVal = findValue(prop);
        if (!propIfcVal) continue;

        if (propName == "AnzahlArbeitsplätze") {

        }

        const auto propVal = getPropValue(propIfcVal);
        pSet[setName][propName] = propVal;
    }
}

void handleQSet(const Ifc4::IfcQuantitySet *set) {
    const auto name = set->Name();
    if (name->empty()) return;

    const auto eq = set->as<Ifc4::IfcElementQuantity>();
    const auto quantities = eq->Quantities();
    for (const auto quantity: (*quantities)) {
        qSet[name.value()][quantity->Name()] = getQuantityValue(quantity);
    }
}

void handleSetDefSelect(const Ifc4::IfcPropertySetDefinitionSelect *select) {
    if (const auto set = select->as<Ifc4::IfcPropertySet>()) {
        handlePSet(set);
        return;
    }

    if (const auto set = select->as<Ifc4::IfcQuantitySet>()) {
        handleQSet(set);
        return;
    }

    std::cerr << "this script doesn't handle " << select->data().type()->name() << std::endl;
}