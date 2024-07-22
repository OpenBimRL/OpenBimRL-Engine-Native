// clang-format off
#include "functions.h"
#include "utils.h"

#include <thread>
#include <ifcgeom_schema_agnostic/IfcGeomIterator.h>
// clang-format on

[[maybe_unused]] void calculatingBuildingBounds() {
  SerializerSettings settings;
  settings.set(IfcGeom::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
  settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
  settings.set(IfcGeom::IteratorSettings::NO_WIRE_INTERSECTION_CHECK, true);
  settings.set(IfcGeom::IteratorSettings::DISABLE_OPENING_SUBTRACTIONS, true);
  settings.set(IfcGeom::IteratorSettings::DISABLE_BOOLEAN_RESULT, true);

  //settings.set(IfcGeom::IteratorSettings::USE_BREP_DATA, true);

  IfcGeom::Iterator geom_iterator(settings,
                                  OpenBimRL::Engine::Utils::getCurrentFile(),
                                  std::thread::hardware_concurrency());

  geom_iterator.initialize();

  geom_iterator.compute_bounds(true);
  const auto lower = geom_iterator.bounds_min();
  const auto higher = geom_iterator.bounds_max();

  // prepare buffer from JVM
  const auto mem_size = 6 * sizeof(Standard_Real);
  auto buffer = OpenBimRL::Engine::Functions::setOutputArray(
      0, mem_size);  // JVM collects garbage automatically
  const Standard_Real values[6] = {
      // construct array
      lower.X(), lower.Y(), lower.Z(), higher.X(), higher.Y(), higher.Z(),
  };

  // copy memory from stack into JVM provided memory
  std::memcpy(buffer, values, mem_size);
}