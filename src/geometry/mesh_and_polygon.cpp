#include "openbimrl/geometry/service.hpp"

#include <array>

namespace OpenBimRL::Native::Geometry {

std::vector<double> footprintPolygonXY(const Model::IfcSession& session,
                                       Model::IfcEntityRef entity) {
    std::vector<double> coords;
    auto geomIterator = iteratorForEntity(session, entity);
    if (!geomIterator->initialize()) return coords;

    IfcGeom::Element* geomObject = nullptr;
    do {
        geomObject = geomIterator->get();
        if (geomObject) break;
    } while (geomIterator->next());
    if (!geomObject) return coords;

    const auto* o = static_cast<const IfcGeom::TriangulationElement*>(geomObject);
    const IfcGeom::Representation::Triangulation& mesh = o->geometry();

    const auto& verticesRaw = mesh.verts();
    const auto verticesCount = verticesRaw.size() / 3;
    std::vector<std::array<double, 2>> vertices;
    vertices.reserve(verticesCount);
    for (std::size_t i = 0; i < verticesCount; i++) {
        const auto idx = i * 3;
        vertices.push_back({verticesRaw[idx], verticesRaw[idx + 1]});
    }

    const auto& facesRaw = mesh.faces();
    const auto facesCount = facesRaw.size() / 3;
    coords.reserve(facesCount * 6);
    for (std::size_t i = 0; i < facesCount; ++i) {
        const auto idx = i * 3;
        for (int k = 0; k < 3; ++k) {
            const auto vi = static_cast<std::size_t>(facesRaw[idx + k]);
            if (vi >= vertices.size()) continue;
            coords.push_back(vertices[vi][0]);
            coords.push_back(vertices[vi][1]);
        }
    }
    return coords;
}

std::vector<Pathfinding::Segment2> extractSegmentsXY(
    const Model::IfcSession& session, Model::IfcEntityRef entity) {
    std::vector<Pathfinding::Segment2> segments;
    auto geomIterator = iteratorForEntity(session, entity);
    if (!geomIterator->initialize()) return segments;

    IfcGeom::Element* geomObject = nullptr;
    do {
        geomObject = geomIterator->get();
        if (geomObject) break;
    } while (geomIterator->next());
    if (!geomObject) return segments;

    const auto* element =
        static_cast<const IfcGeom::TriangulationElement*>(geomObject);
    const IfcGeom::Representation::Triangulation& mesh = element->geometry();

    const auto& verticesRaw = mesh.verts();
    const auto verticesCount = verticesRaw.size() / 3;
    std::vector<std::array<double, 2>> verticesXy;
    verticesXy.reserve(verticesCount);
    for (std::size_t i = 0; i < verticesCount; i++) {
        const auto idx = i * 3;
        verticesXy.push_back({verticesRaw[idx], verticesRaw[idx + 1]});
    }

    const auto& facesRaw = mesh.faces();
    const auto facesCount = facesRaw.size() / 3;
    segments.reserve(facesCount * 3);
    for (std::size_t i = 0; i < facesCount; ++i) {
        const auto idx = i * 3;
        const auto a = static_cast<std::size_t>(facesRaw[idx]);
        const auto b = static_cast<std::size_t>(facesRaw[idx + 1]);
        const auto c = static_cast<std::size_t>(facesRaw[idx + 2]);
        if (a >= verticesXy.size() || b >= verticesXy.size() ||
            c >= verticesXy.size())
            continue;
        const auto& va = verticesXy[a];
        const auto& vb = verticesXy[b];
        const auto& vc = verticesXy[c];
        segments.push_back({va[0], va[1], vb[0], vb[1]});
        segments.push_back({vb[0], vb[1], vc[0], vc[1]});
        segments.push_back({vc[0], vc[1], va[0], va[1]});
    }
    return segments;
}

}  // namespace OpenBimRL::Native::Geometry
