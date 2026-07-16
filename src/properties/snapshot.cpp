#include "openbimrl/properties/data.hpp"

#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3_add2.h>
#include <nlohmann/json.hpp>

#include "ifc_data_schema.h"
#include "ifc_data_utils.h"
#include "openbimrl/model/session.hpp"

namespace OpenBimRL::Native::Properties {

PropertySets g_propertySets;
QuantitySets g_quantitySets;

IfcData snapshot(const Model::IfcSession& session, Model::IfcEntityRef entity) {
    IfcData data;
    if (!entity) return data;

    try {
        const auto* baseConst = static_cast<IfcUtil::IfcBaseClass*>(entity.ptr);
        if (!baseConst) return data;

        auto* base = const_cast<IfcUtil::IfcBaseClass*>(baseConst);
        // Non-throwing cast: invalid pointers must not abort into the JVM.
        const auto* asEntity = base->as<IfcUtil::IfcBaseEntity>(false);
        if (!asEntity) return data;

        data.ifcClass = asEntity->declaration().name();
        try {
            data.GUID = asEntity->get_value<std::string>("GlobalId");
        } catch (const IfcParse::IfcException&) {
            data.GUID.clear();
        }

        g_propertySets.clear();
        g_quantitySets.clear();

        if (session.isIFC4()) {
            populatePropertiesImpl<Ifc4>(base);
        } else if (session.isIFC4X3()) {
            populatePropertiesImpl<Ifc4x3_add2>(base);
        } else if (session.isIFC2x3()) {
            // IFC2x3 property extraction not implemented yet.
        }

        data.propertySets = g_propertySets;
        data.quantitySets = g_quantitySets;
    } catch (const IfcParse::IfcException&) {
        // Leave partially filled data; never propagate to JVM as SIGSEGV.
    } catch (const std::exception&) {
    }
    return data;
}

std::string toJson(const IfcData& data) {
    return nlohmann::json{{"guid", data.GUID},
                          {"ifc_class", data.ifcClass},
                          {"properties", data.propertySets},
                          {"quantities", data.quantitySets}}
        .dump(4);
}

}  // namespace OpenBimRL::Native::Properties
