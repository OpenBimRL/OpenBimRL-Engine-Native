#ifndef OPENBIMRL_NATIVE_PROPERTIES_LEGACY_TYPES_H
#define OPENBIMRL_NATIVE_PROPERTIES_LEGACY_TYPES_H

#include <map>
#include <string>

#include <ifcparse/IfcFile.h>

#include "openbimrl/properties/data.hpp"

namespace OpenBimRL::Native::Properties {

// Working buffers filled by schema populate helpers
extern PropertySets g_propertySets;
extern QuantitySets g_quantitySets;

}  // namespace OpenBimRL::Native::Properties

// Legacy aliases used by schema template headers during migration of IFC4 helpers
namespace OpenBimRL::Engine::Types::IFC {
using IfcObjectPointer = IfcUtil::IfcBaseClass*;
using property = OpenBimRL::Native::Properties::Property;
using quantity = OpenBimRL::Native::Properties::Quantity;
using pSet = OpenBimRL::Native::Properties::PropertySets;
using qSet = OpenBimRL::Native::Properties::QuantitySets;
using IfcData = OpenBimRL::Native::Properties::IfcData;
}  // namespace OpenBimRL::Engine::Types::IFC

#endif  // OPENBIMRL_NATIVE_PROPERTIES_LEGACY_TYPES_H
