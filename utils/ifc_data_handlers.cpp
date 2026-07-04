#include "./ifc_data_schema.h"
#include "./ifc_data_utils.h"

void handlePSet(const Ifc4::IfcPropertySet* set) { handlePSetImpl<Ifc4>(set); }

void handleQSet(const Ifc4::IfcQuantitySet* set) { handleQSetImpl<Ifc4>(set); }

void handleSetDefSelect(const Ifc4::IfcPropertySetDefinitionSelect* select) {
    handleSetDefSelectImpl<Ifc4>(select);
}
