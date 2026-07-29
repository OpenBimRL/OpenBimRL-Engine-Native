#include "openbimrl/geometry/types.hpp"

#include <exception>

#include <ifcparse/Ifc2x3.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3_add2.h>

#include <Standard_Failure.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

namespace OpenBimRL::Native::Geometry {
namespace {

const IfcUtil::IfcBaseEntity* asEntity(const AttributeValue& value) {
    if (value.isNull()) return nullptr;
    IfcUtil::IfcBaseClass* instance = value;
    return instance ? instance->as<IfcUtil::IfcBaseEntity>() : nullptr;
}

gp_Pnt readCartesianPoint(const IfcUtil::IfcBaseEntity* pointEntity) {
    if (!pointEntity) return gp_Pnt(0.0, 0.0, 0.0);

    const auto coords = pointEntity->get("Coordinates");
    if (coords.isNull()) return gp_Pnt(0.0, 0.0, 0.0);

    const auto values = static_cast<std::vector<double>>(coords);
    if (values.size() < 3) {
        return gp_Pnt(values.size() > 0 ? values[0] : 0.0,
                      values.size() > 1 ? values[1] : 0.0, 0.0);
    }
    return gp_Pnt(values[0], values[1], values[2]);
}

gp_Dir readDirection(const IfcUtil::IfcBaseEntity* directionEntity,
                     const gp_Dir& fallback) {
    if (!directionEntity) return fallback;

    const auto ratios = directionEntity->get("DirectionRatios");
    if (ratios.isNull()) return fallback;

    const auto values = static_cast<std::vector<double>>(ratios);
    if (values.empty()) return fallback;

    gp_Vec vec(values.size() > 0 ? values[0] : 0.0,
               values.size() > 1 ? values[1] : 0.0,
               values.size() > 2 ? values[2] : 0.0);
    if (vec.SquareMagnitude() <= gp::Resolution() * gp::Resolution()) {
        return fallback;
    }
    return gp_Dir(vec);
}

gp_Ax3 axis2Placement3DToAx3(const IfcUtil::IfcBaseEntity* placement) {
    if (!placement) return gp_Ax3();

    const auto location = placement->get("Location");
    gp_Pnt origin = location.isNull() ? gp_Pnt()
                                      : readCartesianPoint(asEntity(location));

    gp_Dir axisZ = gp_Dir(0.0, 0.0, 1.0);
    gp_Dir axisX = gp_Dir(1.0, 0.0, 0.0);

    const auto axisAttr = placement->get("Axis");
    if (!axisAttr.isNull()) {
        axisZ = readDirection(asEntity(axisAttr), axisZ);
    }

    const auto refAttr = placement->get("RefDirection");
    if (!refAttr.isNull()) {
        axisX = readDirection(asEntity(refAttr), axisX);
    }

    return gp_Ax3(origin, axisZ, axisX);
}

gp_Ax3 axis2PlacementLinearToAx3(const IfcUtil::IfcBaseEntity* placement) {
    return axis2Placement3DToAx3(placement);
}

gp_Ax3 axis2Placement2DToAx3(const IfcUtil::IfcBaseEntity* placement) {
    if (!placement) return gp_Ax3();

    const auto location = placement->get("Location");
    gp_Pnt origin = location.isNull() ? gp_Pnt()
                                      : readCartesianPoint(asEntity(location));

    gp_Dir axisX = gp_Dir(1.0, 0.0, 0.0);
    const auto refAttr = placement->get("RefDirection");
    if (!refAttr.isNull()) {
        const auto* refEntity = asEntity(refAttr);
        if (refEntity) {
            const auto ratios = refEntity->get("DirectionRatios");
            if (!ratios.isNull()) {
                const auto values = static_cast<std::vector<double>>(ratios);
                if (values.size() >= 2) {
                    gp_Vec vec(values[0], values[1], 0.0);
                    if (vec.SquareMagnitude() >
                        gp::Resolution() * gp::Resolution()) {
                        axisX = gp_Dir(vec);
                    }
                }
            }
        }
    }

    return gp_Ax3(origin, gp_Dir(0.0, 0.0, 1.0), axisX);
}

gp_Ax3 placementRelativeToAx3(const IfcUtil::IfcBaseEntity* relative) {
    if (!relative) return gp_Ax3();

    const auto& decl = relative->declaration().name();
    if (decl == "IfcAxis2Placement3D") {
        return axis2Placement3DToAx3(relative);
    }
    if (decl == "IfcAxis2PlacementLinear") {
        return axis2PlacementLinearToAx3(relative);
    }
    if (decl == "IfcAxis2Placement2D") {
        return axis2Placement2DToAx3(relative);
    }
    return gp_Ax3();
}

gp_Ax3 resolveObjectPlacement(const IfcUtil::IfcBaseEntity* placement) {
    if (!placement) return gp_Ax3();

    const auto& decl = placement->declaration().name();

    if (decl == "IfcLocalPlacement") {
        gp_Ax3 relative = gp_Ax3();
        const auto relativeAttr = placement->get("RelativePlacement");
        if (!relativeAttr.isNull()) {
            relative = placementRelativeToAx3(asEntity(relativeAttr));
        }

        const auto parentAttr = placement->get("PlacementRelTo");
        if (!parentAttr.isNull()) {
            gp_Ax3 parent = resolveObjectPlacement(asEntity(parentAttr));
            // RelativePlacement stores parent-local coordinates in gp_Pnt.
            // SetTransformation(parent, world) converts those into world space:
            //   world = parent.Location + parent.Rotation * local
            // The single-arg SetTransformation(parent) is NOT equivalent and
            // yields ~(-site) for georeferenced models — straights vanished while
            // bboxes (different code path) still appeared on the mesh.
            gp_Trsf parentTrsf;
            parentTrsf.SetTransformation(parent, gp_Ax3());
            relative.Transform(parentTrsf);
        }
        return relative;
    }

    if (decl == "IfcLinearPlacement") {
        gp_Ax3 relative;
        const auto relativeAttr = placement->get("RelativePlacement");
        if (!relativeAttr.isNull()) {
            relative = placementRelativeToAx3(asEntity(relativeAttr));
        } else {
            const auto cartAttr = placement->get("CartesianPosition");
            if (!cartAttr.isNull()) {
                relative = placementRelativeToAx3(asEntity(cartAttr));
            }
        }

        const auto parentAttr = placement->get("PlacementRelTo");
        if (!parentAttr.isNull()) {
            gp_Ax3 parent = resolveObjectPlacement(asEntity(parentAttr));
            gp_Trsf parentTrsf;
            parentTrsf.SetTransformation(parent, gp_Ax3());
            relative.Transform(parentTrsf);
        }
        return relative;
    }

    return gp_Ax3();
}

bool extrusionAxisFromRepresentation(const IfcUtil::IfcBaseEntity* product,
                                     gp_Ax3& out) {
    if (!product) return false;

    try {
        // Entities without a Representation attribute (or with null) must not
        // call into typed Representations() accessors — that segfaults (#6).
        const auto* productDecl = product->declaration().as_entity();
        if (!productDecl || productDecl->attribute_index("Representation") < 0) {
            return false;
        }

        const auto representation = product->get("Representation");
        if (representation.isNull()) return false;

        const auto* representationEntity = asEntity(representation);
        if (!representationEntity) return false;

        const auto* reprDecl = representationEntity->declaration().as_entity();
        if (!reprDecl || reprDecl->attribute_index("Representations") < 0) {
            return false;
        }

        const auto representations =
            representationEntity->get("Representations");
        if (representations.isNull()) return false;

        const auto reprList =
            static_cast<aggregate_of_instance::ptr>(representations);
        if (!reprList || reprList->size() == 0) return false;

        for (auto it = reprList->begin(); it != reprList->end(); ++it) {
            if (!*it) continue;
            const auto shapeRepr = (*it)->as<IfcUtil::IfcBaseEntity>();
            if (!shapeRepr) continue;

            const auto items = shapeRepr->get("Items");
            if (items.isNull()) continue;

            const auto itemList = static_cast<aggregate_of_instance::ptr>(items);
            if (!itemList) continue;

            for (auto itemIt = itemList->begin(); itemIt != itemList->end();
                 ++itemIt) {
                if (!*itemIt) continue;
                const auto item = (*itemIt)->as<IfcUtil::IfcBaseEntity>();
                if (!item ||
                    item->declaration().name() != "IfcExtrudedAreaSolid") {
                    continue;
                }

                const auto position = item->get("Position");
                gp_Ax3 profileAx = position.isNull()
                                       ? gp_Ax3()
                                       : axis2Placement3DToAx3(asEntity(position));

                const auto extrudedDirection = item->get("ExtrudedDirection");
                if (extrudedDirection.isNull()) {
                    out = profileAx;
                    return true;
                }

                gp_Dir extrudeDir = readDirection(asEntity(extrudedDirection),
                                                  profileAx.XDirection());

                out = gp_Ax3(profileAx.Location(), profileAx.Direction(),
                             extrudeDir);
                return true;
            }
        }
    } catch (const IfcParse::IfcException&) {
        return false;
    } catch (const Standard_Failure&) {
        return false;
    } catch (const std::exception&) {
        return false;
    }

    return false;
}

void writeAx3(const gp_Ax3& ax3, ElementFrame& out, FrameSource source) {
    const gp_Pnt loc = ax3.Location();
    const gp_Dir xDir = ax3.XDirection();
    const gp_Dir zDir = ax3.Direction();

    out.point[0] = loc.X();
    out.point[1] = loc.Y();
    out.point[2] = loc.Z();
    out.axisX[0] = xDir.X();
    out.axisX[1] = xDir.Y();
    out.axisX[2] = xDir.Z();
    out.axisZ[0] = zDir.X();
    out.axisZ[1] = zDir.Y();
    out.axisZ[2] = zDir.Z();
    out.source = source;
}

bool tryPlacementFrame(const IfcUtil::IfcBaseEntity* product,
                       ElementFrame& out) {
    const auto placementAttr = product->get("ObjectPlacement");
    if (placementAttr.isNull()) return false;

    const auto* placement = asEntity(placementAttr);
    if (!placement) return false;

    writeAx3(resolveObjectPlacement(placement), out, FrameSource::PLACEMENT);
    return true;
}

bool tryRepresentationFrame(const IfcUtil::IfcBaseEntity* product,
                            ElementFrame& out) {
    gp_Ax3 ax3;
    if (!extrusionAxisFromRepresentation(product, ax3)) return false;

    writeAx3(ax3, out, FrameSource::REPRESENTATION);
    return true;
}

}  // namespace

bool extractFrame(Model::IfcEntityRef entity, ElementFrame& out) {
    if (!entity) return false;

    try {
        const auto* base = static_cast<IfcUtil::IfcBaseClass*>(entity.ptr);
        if (!base) return false;

        const auto* product = base->as<IfcUtil::IfcBaseEntity>(false);
        if (!product) return false;

        if (tryPlacementFrame(product, out)) return true;
        if (tryRepresentationFrame(product, out)) return true;

        out.source = FrameSource::NONE;
        return false;
    } catch (const IfcParse::IfcException&) {
        out.source = FrameSource::NONE;
        return false;
    } catch (const Standard_Failure&) {
        out.source = FrameSource::NONE;
        return false;
    } catch (const std::exception&) {
        out.source = FrameSource::NONE;
        return false;
    }
}

}  // namespace OpenBimRL::Native::Geometry
