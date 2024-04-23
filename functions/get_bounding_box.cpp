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

  if (Utils::isIFC4()) {
    const auto spatial = ifcObject->as<Ifc4::IfcSpatialStructureElement>();

    if (!spatial) return;
    const auto representation = spatial->Representation();

    IfcGeom::IteratorSettings settings;
    settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
    settings.set(IfcGeom::IteratorSettings::WELD_VERTICES, false);
    settings.set(IfcGeom::IteratorSettings::DISABLE_TRIANGULATION, false);
    settings.set(IfcGeom::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
    settings.set(IfcGeom::IteratorSettings::NO_NORMALS, true);

    const auto representation_list = representation->Representations();
    const auto some_representation = *(representation_list->begin());

    // copying this is not optimal but there are what? 60 doubles? I think
    // that's manageable
    const auto element =
        Utils::create_shape(settings, spatial, some_representation);

    Bnd_Box bbox;

    BRepBndLib::AddOptimal(element, bbox);
    const auto corner_min = bbox.CornerMin(), corner_max = bbox.CornerMax();

    const auto mem_size = 6 * sizeof(Standard_Real);
    auto buffer = Functions::setOutputArray(0, mem_size);
    const Standard_Real values[6] = {
        corner_min.X(), corner_min.Y(), corner_min.Z(),
        corner_max.X(), corner_max.Y(), corner_max.Z(),
    };

    std::memcpy(buffer, values, mem_size);
  }
}