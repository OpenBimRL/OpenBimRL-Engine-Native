#ifndef OPENBIMRL_NATIVE_PROPERTIES_DATA_H
#define OPENBIMRL_NATIVE_PROPERTIES_DATA_H

#include <map>
#include <string>

#include "openbimrl/model/entity.hpp"
#include "openbimrl/model/session.hpp"

namespace OpenBimRL::Native::Properties {

using Property = std::map<std::string, std::string>;
using Quantity = std::map<std::string, double>;
using PropertySets = std::map<std::string, Property>;
using QuantitySets = std::map<std::string, Quantity>;

struct IfcData {
    std::string ifcClass;
    std::string GUID;
    PropertySets propertySets;
    QuantitySets quantitySets;
};

IfcData snapshot(const Model::IfcSession& session, Model::IfcEntityRef entity);
std::string toJson(const IfcData& data);

}  // namespace OpenBimRL::Native::Properties

#endif  // OPENBIMRL_NATIVE_PROPERTIES_DATA_H
