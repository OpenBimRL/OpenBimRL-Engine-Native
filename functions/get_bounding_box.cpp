#include <ifcparse/Ifc4.h>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#include "functions.h"
#include "utils.h"


[[maybe_unused]] void getBoundingBox(void) {
  using namespace OpenBimRL::Engine;
  const auto ifcObject =
      (Types::IFC::IfcObjectPointer)Functions::getInputPointer(0);
  if (!ifcObject) return;

  if (Utils::isIFC4()) { // currently only works with IFC4
    const auto spatial = ifcObject->as<Ifc4::IfcSpatialStructureElement>();
    // check if element is of type IfcSpatialStructureElement (IfcSpace, IfcSite, ...) could probably expanded to any IfcElement with geometry

    if (!spatial) return;
    const auto representation = spatial->Representation();

    IfcGeom::IteratorSettings settings; // configure geometry generator settings
    settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
    settings.set(IfcGeom::IteratorSettings::WELD_VERTICES, false);
    settings.set(IfcGeom::IteratorSettings::DISABLE_TRIANGULATION, false);
    settings.set(IfcGeom::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
    settings.set(IfcGeom::IteratorSettings::NO_NORMALS, true); // we don't need normals for simple point geometry

    const auto representation_list = representation->Representations();
    const auto some_representation = *(representation_list->begin()); // take the first (any most of the times the only) representation

    // copying this is not optimal but there are what? 60 doubles? I think
    // that's manageable
    const auto element =
        Utils::create_shape(settings, spatial, some_representation);

    Bnd_Box bbox;

    BRepBndLib::AddOptimal(element, bbox); // automatically expands the box to fit given geometry by adding points util every point is inside the box
    const auto corner_min = bbox.CornerMin(), corner_max = bbox.CornerMax();

    // prepare buffer from JVM
    const auto mem_size = 6 * sizeof(Standard_Real);
    auto buffer = Functions::setOutputArray(0, mem_size); // JVM collects garbage automatically
    const Standard_Real values[6] = { // construct array
        corner_min.X(), corner_min.Y(), corner_min.Z(),
        corner_max.X(), corner_max.Y(), corner_max.Z(),
    };

    // copy memory from stack into JVM provided memory
    std::memcpy(buffer, values, mem_size);
  }
}
