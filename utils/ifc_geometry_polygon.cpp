#include <TopoDS_Vertex.hxx>

#include "lib.h"
#include "utils.h"

static std::vector<Standard_Real> coords_buffer;

std::size_t request_geometry_polygon(JNA::Pointer ifcPointer) {
  using namespace OpenBimRL::Engine;

  const auto ifcObject = static_cast<Types::IFC::IfcObjectPointer>(ifcPointer);

  const auto topoCompound = Utils::create_shape_default(ifcObject);
  if (!topoCompound.has_value()) return -1;


  coords_buffer.clear();

  for(TopExp_Explorer anExp(topoCompound.value(), TopAbs_VERTEX); anExp.More(); anExp.Next()) {
    TopoDS_Vertex vertex = TopoDS::Vertex(anExp.Current());
    gp_Pnt pt = BRep_Tool::Pnt(vertex);

    coords_buffer.push_back(pt.X());
    coords_buffer.push_back(pt.Y());
    coords_buffer.push_back(pt.Z());
  }


  return coords_buffer.size();
}

void copy_geometry_polygon(JNA::Pointer buffer) {
  std::memcpy(buffer, coords_buffer.data(), coords_buffer.size());
}