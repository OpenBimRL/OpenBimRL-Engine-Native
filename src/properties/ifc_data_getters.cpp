#include "./ifc_data_schema.h"
#include "./ifc_data_utils.h"

double getQuantityValue(const Ifc4::IfcPhysicalQuantity* quantity) {
    return getQuantityValueImpl<Ifc4>(quantity);
}

std::string getPropValue(const Ifc4::IfcValue* prop) {
    return getPropValueImpl<Ifc4>(prop);
}
