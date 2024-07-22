#ifndef OPENBIMRL_NATIVE_IFC_TYPEDEFS_H
#define OPENBIMRL_NATIVE_IFC_TYPEDEFS_H

#include <ifcparse/IfcFile.h>

namespace OpenBimRL::Engine::Types::IFC {
typedef IfcUtil::IfcBaseClass *IfcObjectPointer;
typedef std::map<std::string, std::string> property;
typedef std::map<std::string, double> quantity;
typedef std::map<std::string, property> pSet;
typedef std::map<std::string, quantity> qSet;
typedef struct {
    std::string ifcClass;
    std::string GUID;
    pSet propertySets;
    qSet quantitySets;
} IfcData;
}  // namespace OpenBimRL::Engine::Types::IFC
#endif  // OPENBIMRL_NATIVE_IFC_TYPEDEFS_H
