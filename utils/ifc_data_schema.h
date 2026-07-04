#ifndef OPENBIMRL_NATIVE_IFC_DATA_SCHEMA_H
#define OPENBIMRL_NATIVE_IFC_DATA_SCHEMA_H

#include <iostream>
#include <sstream>
#include <vector>

#include "types.h"

extern OpenBimRL::Engine::Types::IFC::pSet pSet;
extern OpenBimRL::Engine::Types::IFC::qSet qSet;

template <typename Schema>
typename Schema::IfcValue* findValueImpl(
    const typename Schema::IfcProperty* property) {
    if (const auto item = property->template as<typename Schema::IfcPropertySingleValue>())
        return item->NominalValue();

    if (const auto item =
            property->template as<typename Schema::IfcPropertyEnumeratedValue>()) {
        const auto optional = item->EnumerationValues();
        if (!optional.has_value()) return nullptr;

        const auto& list = optional.get();
        if (!list->size()) return nullptr;

        if (const auto label =
                (*(list->begin()))->template as<typename Schema::IfcLabel>())
            return label;

        return nullptr;
    }

    return nullptr;
}

template <typename Schema>
std::string getPropValueImpl(const typename Schema::IfcValue* prop) {
    std::stringstream ss;

    if (const auto item = prop->template as<typename Schema::IfcLabel>()) {
        ss << static_cast<std::string>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcIdentifier>()) {
        ss << static_cast<std::string>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcText>()) {
        ss << static_cast<std::string>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcBoolean>()) {
        ss << static_cast<bool>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcInteger>()) {
        ss << static_cast<int>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcReal>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcCountMeasure>()) {
        ss << static_cast<int>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcVolumeMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcTimeMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcThermodynamicTemperatureMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcSolidAngleMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcPositiveRatioMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcRatioMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcPositivePlaneAngleMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcPlaneAngleMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcParameterValue>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcNumericMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcMassMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcPositiveLengthMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcLengthMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcElectricCurrentMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcDescriptiveMeasure>()) {
        ss << static_cast<std::string>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcContextDependentMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcAreaMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcAmountOfSubstanceMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcLuminousIntensityMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcNormalisedRatioMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item =
            prop->template as<typename Schema::IfcNonNegativeLengthMeasure>()) {
        ss << static_cast<double>(*item);
        return ss.str();
    }
    if (const auto item = prop->template as<typename Schema::IfcComplexNumber>()) {
        const auto values = static_cast<std::vector<double>>(*item);
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i > 0) ss << ',';
            ss << values[i];
        }
        return ss.str();
    }

    return "";
}

template <typename Schema>
double getQuantityValueImpl(const typename Schema::IfcPhysicalQuantity* quantity) {
    if (const auto item = quantity->template as<typename Schema::IfcQuantityArea>()) {
        return item->AreaValue();
    }
    if (const auto item = quantity->template as<typename Schema::IfcQuantityCount>()) {
        return item->CountValue();
    }
    if (const auto item = quantity->template as<typename Schema::IfcQuantityLength>()) {
        return item->LengthValue();
    }
    if (const auto item = quantity->template as<typename Schema::IfcQuantityTime>()) {
        return item->TimeValue();
    }
    if (const auto item = quantity->template as<typename Schema::IfcQuantityVolume>()) {
        return item->VolumeValue();
    }
    if (const auto item = quantity->template as<typename Schema::IfcQuantityWeight>()) {
        return item->WeightValue();
    }
    return -1.0;
}

template <typename Schema>
void handlePSetImpl(const typename Schema::IfcPropertySet* set) {
    if (set->Name()->empty()) return;

    const auto setName = set->Name().value();
    const auto props = set->HasProperties();

    for (const auto prop : (*props)) {
        const auto propName = prop->Name();
        const auto propIfcVal = findValueImpl<Schema>(prop);
        if (!propIfcVal) continue;

        const auto propVal = getPropValueImpl<Schema>(propIfcVal);
        pSet[setName][propName] = propVal;
    }
}

template <typename Schema>
void handleQSetImpl(const typename Schema::IfcQuantitySet* set) {
    const auto name = set->Name();
    if (name->empty()) return;

    const auto eq = set->template as<typename Schema::IfcElementQuantity>();
    const auto quantities = eq->Quantities();
    for (const auto quantity : (*quantities)) {
        qSet[name.value()][quantity->Name()] = getQuantityValueImpl<Schema>(quantity);
    }
}

template <typename Schema>
void handleSetDefSelectImpl(
    const typename Schema::IfcPropertySetDefinitionSelect* select) {
    if (const auto set = select->template as<typename Schema::IfcPropertySet>()) {
        handlePSetImpl<Schema>(set);
        return;
    }

    if (const auto set = select->template as<typename Schema::IfcQuantitySet>()) {
        handleQSetImpl<Schema>(set);
        return;
    }

    std::cerr << "this script doesn't handle " << select->declaration().name()
              << std::endl;
}

template <typename Schema>
void populatePropertiesImpl(OpenBimRL::Engine::Types::IFC::IfcObjectPointer ifcObject) {
    const auto product = ifcObject->template as<typename Schema::IfcObject>();
    const auto definitions = product->IsDefinedBy();
    for (const auto definition : (*definitions)) {
        if (const auto byProps =
                definition->template as<typename Schema::IfcRelDefinesByProperties>()) {
            const auto propDefs = byProps->RelatingPropertyDefinition();
            handleSetDefSelectImpl<Schema>(propDefs);
            continue;
        }
        if (const auto byType =
                definition->template as<typename Schema::IfcRelDefinesByType>()) {
            const auto relatingType = byType->RelatingType();
            const auto propertySetsOptional = relatingType->HasPropertySets();
            if (!propertySetsOptional.has_value()) continue;

            for (const auto propertySet : (*propertySetsOptional.value())) {
                handleSetDefSelectImpl<Schema>(propertySet);
            }
        }
    }
}

#endif  // OPENBIMRL_NATIVE_IFC_DATA_SCHEMA_H
