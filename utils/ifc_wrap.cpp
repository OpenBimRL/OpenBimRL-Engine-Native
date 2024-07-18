#include <boost/make_shared.hpp>

#include "utils.h"

/*
 * This code was mainly extracted from IfcWrap (therefore the name)
 * I don't know how it works and I really don't want to know...
 *
 * All rights to ifcopenshell for this one :)
 */

template <typename T>
static std::string to_locale_invariant_string(const T& t) {
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << t;
    return oss.str();
}

template <typename Schema>
static std::optional<boost::shared_ptr<IfcGeom::Representation::BRep>>
helper_fn_create_shape(IfcGeom::IteratorSettings& settings,
                       IfcUtil::IfcBaseClass* instance,
                       IfcUtil::IfcBaseClass* representation = nullptr) {
    IfcGeom::Kernel kernel(OpenBimRL::Engine::Utils::getCurrentFile());

#pragma region KernelSettings
    kernel.setValue(IfcGeom::Kernel::GV_MAX_FACES_TO_ORIENT,
                    settings.get(IfcGeom::IteratorSettings::SEW_SHELLS)
                        ? std::numeric_limits<double>::infinity()
                        : -1);
    kernel.setValue(
        IfcGeom::Kernel::GV_DIMENSIONALITY,
        (settings.get(IfcGeom::IteratorSettings::INCLUDE_CURVES)
             ? (settings.get(
                    IfcGeom::IteratorSettings::EXCLUDE_SOLIDS_AND_SURFACES)
                    ? -1.
                    : 0.)
             : +1.));
    kernel.setValue(
        IfcGeom::Kernel::GV_LAYERSET_FIRST,
        settings.get(IfcGeom::IteratorSettings::LAYERSET_FIRST) ? +1.0 : -1.0);
    kernel.setValue(
        IfcGeom::Kernel::GV_NO_WIRE_INTERSECTION_CHECK,
        settings.get(IfcGeom::IteratorSettings::NO_WIRE_INTERSECTION_CHECK)
            ? +1.0
            : -1.0);
    kernel.setValue(
        IfcGeom::Kernel::GV_NO_WIRE_INTERSECTION_TOLERANCE,
        settings.get(IfcGeom::IteratorSettings::NO_WIRE_INTERSECTION_TOLERANCE)
            ? +1.0
            : -1.0);
    kernel.setValue(
        IfcGeom::Kernel::GV_PRECISION_FACTOR,
        settings.get(IfcGeom::IteratorSettings::STRICT_TOLERANCE) ? 1.0 : 10.0);

    kernel.setValue(
        IfcGeom::Kernel::GV_DISABLE_BOOLEAN_RESULT,
        settings.get(IfcGeom::IteratorSettings::DISABLE_BOOLEAN_RESULT) ? +1.0
                                                                        : -1.0);

    kernel.setValue(
        IfcGeom::Kernel::GV_DEBUG_BOOLEAN,
        settings.get(IfcGeom::IteratorSettings::DEBUG_BOOLEAN) ? +1.0 : -1.0);

    kernel.setValue(IfcGeom::Kernel::GV_BOOLEAN_ATTEMPT_2D,
                    settings.get(IfcGeom::IteratorSettings::BOOLEAN_ATTEMPT_2D)
                        ? +1.0
                        : -1.0);

#pragma endregion KernelSettings

#pragma region Unhappy_Path
    if (!instance->declaration().is(Schema::IfcProduct::Class())) {
        if (representation) {
            std::cout << "Invalid additional representation specified"
                      << std::endl;
            return std::nullopt;
        }

        if (!instance->declaration().is(
                Schema::IfcRepresentationItem::Class()) &&
            !instance->declaration().is(Schema::IfcRepresentation::Class()) &&
            !instance->declaration().is(Schema::IfcProfileDef::Class()))
            return std::nullopt;

        IfcGeom::IfcRepresentationShapeItems shapes = kernel.convert(instance);

        IfcGeom::ElementSettings element_settings(
            settings, kernel.getValue(IfcGeom::Kernel::GV_LENGTH_UNIT),
            instance->declaration().name());
        return {boost::make_shared<IfcGeom::Representation::BRep>(
            element_settings, to_locale_invariant_string(instance->data().id()),
            shapes)};
    }
#pragma endregion Unhappy_Path

#pragma region Happy_Path

    // check if representation given is right type
    if (representation &&
        !representation->declaration().is(Schema::IfcRepresentation::Class())) {
        std::cout << "Supplied representation not of type IfcRepresentation"
                  << std::endl;
        return std::nullopt;
    }

    const auto product = static_cast<typename Schema::IfcProduct*>(instance);

    if (!representation && !product->Representation()) {
        std::cout << "Product has no representation" << std::endl;
        return std::nullopt;
    }

    typename Schema::IfcProductRepresentation* product_representation =
        product->Representation();
    typename Schema::IfcRepresentation::list::ptr reps =
        product_representation->Representations();
    auto* ifc_representation =
        (typename Schema::IfcRepresentation*)representation;

    if (!ifc_representation) {
        // First, try to find a representation based on the settings
        for (auto it = reps->begin(); it != reps->end(); ++it) {
            typename Schema::IfcRepresentation* rep = *it;
            if (!rep->RepresentationIdentifier()) {
                continue;
            }
            if (!settings.get(
                    IfcGeom::IteratorSettings::EXCLUDE_SOLIDS_AND_SURFACES)) {
                if (*rep->RepresentationIdentifier() == "Body") {
                    ifc_representation = rep;
                    break;
                }
            }
            if (settings.get(IfcGeom::IteratorSettings::INCLUDE_CURVES)) {
                if (*rep->RepresentationIdentifier() == "Plan" ||
                    *rep->RepresentationIdentifier() == "Axis") {
                    ifc_representation = rep;
                    break;
                }
            }
        }
    }

    // Otherwise, find a representation within the 'Model' or 'Plan' context
    if (!ifc_representation) {
        for (auto it = reps->begin(); it != reps->end(); ++it) {
            typename Schema::IfcRepresentation* rep = *it;
            typename Schema::IfcRepresentationContext* context =
                rep->ContextOfItems();

            // TODO: Remove redundancy with IfcGeomIterator.h
            if (context->ContextType()) {
                std::set<std::string> context_types;
                if (!settings.get(IfcGeom::IteratorSettings::
                                      EXCLUDE_SOLIDS_AND_SURFACES)) {
                    context_types.insert("model");
                    context_types.insert("design");
                    context_types.insert("model view");
                    context_types.insert("detail view");
                }
                if (settings.get(IfcGeom::IteratorSettings::INCLUDE_CURVES)) {
                    context_types.insert("plan");
                }

                std::string context_type_lc = *context->ContextType();
                for (char& c : context_type_lc) {
                    c = (char)tolower(c);
                }
                if (context_types.find(context_type_lc) !=
                    context_types.end()) {
                    ifc_representation = rep;
                }
            }
        }
    }

    if (!ifc_representation) {
        if (reps->size()) {
            // Return a random representation
            ifc_representation = *reps->begin();
        } else {
            std::cout << "No suitable IfcRepresentation found" << std::endl;
            return std::nullopt;
        }
    }

    // Read precision for found representation's context
    auto context = ifc_representation->ContextOfItems();
    if (context->template as<
            typename Schema::IfcGeometricRepresentationSubContext>()) {
        context =
            context
                ->template as<
                    typename Schema::IfcGeometricRepresentationSubContext>()
                ->ParentContext();
    }
    if (context->template as<
            typename Schema::IfcGeometricRepresentationContext>() &&
        context
            ->template as<typename Schema::IfcGeometricRepresentationContext>()
            ->Precision()) {
        double p =
            *context
                 ->template as<
                     typename Schema::IfcGeometricRepresentationContext>()
                 ->Precision() *
            kernel.getValue(IfcGeom::Kernel::GV_PRECISION_FACTOR);
        p *= kernel.getValue(IfcGeom::Kernel::GV_LENGTH_UNIT);
        if (p < 1.e-7) {
            Logger::Message(
                Logger::LOG_WARNING,
                "Precision lower than 0.0000001 meter not enforced");
            p = 1.e-7;
        }
        kernel.setValue(IfcGeom::Kernel::GV_PRECISION, p);
    }

    IfcGeom::BRepElement* b_representation =
        kernel.convert(settings, ifc_representation, product);
    if (!b_representation) {
        std::cout << "Failed to process shape" << std::endl;
        return std::nullopt;
    }

    return b_representation->geometry_pointer();

#pragma endregion Happy_Path
}

std::optional<boost::shared_ptr<IfcGeom::Representation::BRep>>
OpenBimRL::Engine::Utils::create_shape_default(
    IfcUtil::IfcBaseClass* ifcObject) {
    // configure geometry generator settings
    IfcGeom::IteratorSettings settings;
    settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
//    settings.set(IfcGeom::IteratorSettings::WELD_VERTICES, true);
//    settings.set(IfcGeom::IteratorSettings::DISABLE_OPENING_SUBTRACTIONS, true);
//    settings.set(IfcGeom::IteratorSettings::APPLY_LAYERSETS, true);
//    settings.set(IfcGeom::IteratorSettings::SITE_LOCAL_PLACEMENT, true);
//    settings.set(IfcGeom::IteratorSettings::BUILDING_LOCAL_PLACEMENT, true);
//    settings.set(IfcGeom::IteratorSettings::SEW_SHELLS, true);

    if (isIFC4()) {
        const auto product = ifcObject->as<Ifc4::IfcProduct>();

        if (!product) return std::nullopt;

        return {create_shape(settings, product)};
    } else if (isIFC2x3()) {
        const auto product = ifcObject->as<Ifc2x3::IfcProduct>();

        if (!product) return std::nullopt;

        return {create_shape(settings, product)};
    }

    return std::nullopt;
}

std::optional<boost::shared_ptr<IfcGeom::Representation::BRep>>
OpenBimRL::Engine::Utils::create_shape(IfcGeom::IteratorSettings& settings,
                                       IfcUtil::IfcBaseClass* instance,
                                       IfcUtil::IfcBaseClass* representation) {
    if (isIFC2x3()) {
        return helper_fn_create_shape<Ifc2x3>(settings, instance,
                                              representation);
    }

    if (isIFC4()) {
        return helper_fn_create_shape<Ifc4>(settings, instance, representation);
    }

    throw IfcParse::IfcException("No geometry support for " +
                                 instance->declaration().schema()->name());
}