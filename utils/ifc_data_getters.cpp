#include "./ifc_data_utils.h"

double getQuantityValue(const Ifc4::IfcPhysicalQuantity *quantity) {
    if (const auto item = quantity->as<Ifc4::IfcQuantityArea>()) {
        return item->AreaValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityCount>()) {
        return item->CountValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityLength>()) {
        return item->LengthValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityTime>()) {
        return item->TimeValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityVolume>()) {
        return item->VolumeValue();
    }
    if (const auto item = quantity->as<Ifc4::IfcQuantityWeight>()) {
        return item->WeightValue();
    }
    return -1.0;
}

std::string getPropValue(const Ifc4::IfcValue *prop) {
    std::stringstream ss;

    if (const auto item = prop->as<Ifc4::IfcLabel>()) {
        ss << item->operator std::string();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcIdentifier>()) {
        ss << item->operator std::string();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcText>()) {
        ss << item->operator std::string();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcBoolean>()) {
        ss << (item->operator bool() ? "true" : "false");
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcInteger>()) {
        ss << item->operator int();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcReal>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcCountMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcVolumeMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcTimeMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item =
            prop->as<Ifc4::IfcThermodynamicTemperatureMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcSolidAngleMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcPositiveRatioMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcRatioMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcPositivePlaneAngleMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcPlaneAngleMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcParameterValue>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcNumericMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcMassMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcPositiveLengthMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcLengthMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcElectricCurrentMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcDescriptiveMeasure>()) {
        ss << item->operator std::string();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcCountMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcContextDependentMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcAreaMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcAmountOfSubstanceMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcLuminousIntensityMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcNormalisedRatioMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcNonNegativeLengthMeasure>()) {
        ss << item->operator double();
        return ss.str();
    }
    if (const auto item = prop->as<Ifc4::IfcComplexNumber>()) {
        throw std::runtime_error("not implemented");
    }

    return "";
}