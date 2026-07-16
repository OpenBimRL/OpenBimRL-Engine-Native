#ifndef OPENBIMRL_NATIVE_IFC_DATA_UTILS_H
#define OPENBIMRL_NATIVE_IFC_DATA_UTILS_H

#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3_add2.h>

#include "openbimrl/properties/legacy_types.hpp"

namespace OpenBimRL::Native::Properties {
extern PropertySets g_propertySets;
extern QuantitySets g_quantitySets;
}  // namespace OpenBimRL::Native::Properties

void handleSetDefSelect(const Ifc4::IfcPropertySetDefinitionSelect* select);

void handleQSet(const Ifc4::IfcQuantitySet* set);
void handlePSet(const Ifc4::IfcPropertySet* set);

std::string getPropValue(const Ifc4::IfcValue* prop);
double getQuantityValue(const Ifc4::IfcPhysicalQuantity* quantity);

Ifc4::IfcValue* handleEnumeratedProperty(
    boost::optional<boost::shared_ptr<Ifc4::IfcValue::list>> optional);
Ifc4::IfcValue* findValue(const Ifc4::IfcProperty* property);

#endif  // OPENBIMRL_NATIVE_IFC_DATA_UTILS_H
