#ifndef OPENBIMRL_NATIVE_IFC_TYPEDEFS_H
#define OPENBIMRL_NATIVE_IFC_TYPEDEFS_H

#include <ifcparse/IfcFile.h>

namespace OpenBimRL::Engine::Types::IFC {
    typedef IfcUtil::IfcBaseClass *IfcObjectPointer;
    typedef std::map<std::string, std::string> property;
    typedef std::map<std::string, property> property_sets;
    typedef struct {
        std::string name;
        std::string type;
        std::string GUID;
        property_sets propertySets;
    } IfcData;
}
#endif // OPENBIMRL_NATIVE_IFC_TYPEDEFS_H
