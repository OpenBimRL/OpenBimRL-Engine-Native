#include "./ifc_data_utils.h"

Ifc4::IfcValue *handleEnumeratedProperty(const boost::optional<boost::shared_ptr<Ifc4::IfcValue::list>> optional)
{
    if (!optional.has_value())
        return nullptr;

    const auto& list = optional.get();
    if (!list->size())
        return nullptr;

    if (const auto item = (*(list->begin()))->as<Ifc4::IfcLabel>())
        return item;

    return nullptr;

}