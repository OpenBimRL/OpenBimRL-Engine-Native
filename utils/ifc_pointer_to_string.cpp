#include "lib.h"
#include "utils.h"

#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc2x3.h>

/// @brief writes IfcType#GUID to stringPointer
/// @param ifcObject only works if GUID is at 0th position
/// @param stringPointer pointer to free memory (ideally zeros)
/// @param maxLen string maximum size
[[maybe_unused]]
bool ifc_object_to_string(JNA::Pointer ifcObject, JNA::Pointer stringPointer, const std::size_t maxLen)
{
    if (!ifcObject)
        return false;

    const auto base = static_cast<IfcUtil::IfcBaseClass *>(ifcObject);

    // remove quotes from string
    auto guid = OpenBimRL::Engine::Utils::isIFC4() ?
            dynamic_cast<Ifc4::IfcObject *>(base)->GlobalId() :
            dynamic_cast<Ifc2x3::IfcObject *>(base)->GlobalId();

    std::stringstream ss;
    ss << base->data().type()->name() << '#' << guid;
    const auto str = ss.str();
    std::memcpy(stringPointer, str.c_str(), std::min(maxLen, str.size()));

    return true;
}