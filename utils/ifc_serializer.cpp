#include "lib.h"
#include "utils.h"

#include <nlohmann/json.hpp>

static std::string buffer = std::string();

[[maybe_unused]] std::size_t request_ifc_object_json_size(JNA::Pointer ifcPointer) {
    buffer.clear();
    const auto ifcObject = (IFC::IfcObjectPointer)ifcPointer;
    auto data = OpenBimRL::Engine::Utils::getData(ifcObject);
    OpenBimRL::Engine::Utils::populateProperties(data, ifcObject);

    const auto json = nlohmann::json{
            {"guid",       data.GUID},
            {"ifc_class",  data.ifcClass},
            {"properties", data.propertySets},
            {"quantities", data.quantitySets}
    }.dump(4);

    buffer.append(json);

    return std::strlen(buffer.c_str()) + 1;
}

[[maybe_unused]] void ifc_object_to_json(JNA::Pointer buff) {
    const auto str = buffer.c_str();
    std::memcpy(buff, str, std::strlen(str) + 1);
}