#include "functions.h"
#include "utils.h"

#include <ifcparse/Ifc4.h>

[[maybe_unused]] void getBoundingBox(void) {
  using namespace OpenBimRL::Engine;
  const auto ifcObject = (Types::IFC::IfcObjectPointer)Functions::getInputPointer(0);
  if (!ifcObject) return;

  if (Utils::isIFC4()) {
    const auto spatial = ifcObject->as<Ifc4::IfcSpatialStructureElement>();
    if (!spatial) return;

    // const auto geometry = OpenBimRL::Engine::Utils::getKernel()->convert(spatial);
/*    const auto representationDefinition = spatial->Representation();
    const auto representations = representationDefinition->Representations();

    if (!representations->size()) return;

    for (const auto representation : *representations) {
      const auto model = representation->as<Ifc4::IfcShapeRepresentation>();
      if (!model) continue;

      const auto items = model->Items();

      for (const auto item : *(items)) {
        const auto solid = item->as<Ifc4::IfcSweptAreaSolid>();
        const auto test = nullptr;
      }
    }*/
  }
}