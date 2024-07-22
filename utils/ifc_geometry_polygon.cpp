#include <math_Vector.hxx>

#include "lib.h"
#include "utils.h"

#define NORMAL_TOLERANCE 0.01
static std::vector<double> coords_buffer;

void transformToVector(const std::vector<double>&, std::vector<gp_Vec>&);

std::size_t request_geometry_polygon(JNA::Pointer ifcPointer) {
    coords_buffer.clear();

    using namespace OpenBimRL::Engine;

    const auto ifcObject =
        static_cast<Types::IFC::IfcObjectPointer>(ifcPointer);

    const auto optionalBRep = Utils::create_shape_default(ifcObject);
    if (!optionalBRep.has_value()) return -1;

    const auto& bRepPtr = optionalBRep.value();

    IfcGeom::Representation::Triangulation triangulation(*bRepPtr);

    const gp_Dir axis(0., 0., 1.);

    const auto& verticesRaw = triangulation.verts();
    const auto& facesRaw = triangulation.faces();
    const auto facesCount = facesRaw.size() / 3;

    std::vector<gp_Vec> vertices;
    transformToVector(verticesRaw, vertices);

    std::vector<std::array<int, 3>> faces;
    faces.reserve(facesCount);

    for (std::uint32_t i = 0; i < facesCount; ++i) {
        const auto actualI = i * 3;
        faces.push_back(
            {facesRaw[actualI], facesRaw[actualI + 1], facesRaw[actualI + 2]});
    }


    std::vector<gp_Vec> triangleNormals;

    for (const auto face : faces) {
        const auto fn1 = vertices[face[1]] - vertices[face[0]];
        const auto fn2 = vertices[face[2]] - vertices[face[0]];
        try {
            triangleNormals.push_back(fn1.Crossed(fn2).Normalized());
        } catch (Standard_ConstructionError &e) {
            // ignore me
        }
    }

    std::vector<std::array<int, 3>> filtered_faces;

    for (std::uint32_t i = 0; i < triangleNormals.size(); ++i) {
        if (triangleNormals[i] * axis > NORMAL_TOLERANCE) continue;
        filtered_faces.push_back(faces[i]);
    }

    std::vector<gp_Vec> flattenedVertices;
    flattenedVertices.reserve(vertices.size());
    for (auto currentVertex : vertices)
        flattenedVertices.push_back(currentVertex -
                                    ((currentVertex * axis) * axis));

    const auto d = gp_Vec(axis) / math_Vector(axis.XYZ()).Norm2();
    auto a = d;  // I don't know what these lines are doing ¯\_(ツ)_/¯
    // ask them:
    // https://github.com/IfcOpenShell/IfcOpenShell/blob/447a13b6e7552d69cf8abc2531fe52a45f6acd43/src/ifcopenshell-python/ifcopenshell/util/shape.py#L646
    if (d.Z() - 1. > 0.01)
        a.SetCoord(2, a.Z() + 0.01);
    else
        a = gp_Vec(1., 0., 0.);

    // first basis vector
    auto b = d ^ a;
    b /= math_Vector(b.XYZ()).Norm2();

    // second basis vector
    const auto c = d ^ b;

    // Project the flattened vertices onto the basis to get 2D coordinates

    std::vector<gp_Vec2d> vertices2D;
    for (const auto vertex : vertices)
        vertices2D.emplace_back(vertex * b, vertex * c);

    coords_buffer.reserve(filtered_faces.size() * 3);

    for (const auto face : filtered_faces) {
        for (std::uint32_t i = 0; i < 3; ++i) {
            const auto tempVertex = vertices2D[face[i]];
            coords_buffer.push_back(tempVertex.X());
            coords_buffer.push_back(tempVertex.Y());
        }
    }

/*    for (const auto face : faces) {
        for (const auto index : face) {
            const auto vertex = vertices[index];
            coords_buffer.push_back(vertex.X());
            coords_buffer.push_back(vertex.Z());
        }
    }*/

    return coords_buffer.size();
}

[[maybe_unused]] void copy_geometry_polygon(JNA::Pointer buffer) {
    std::memcpy(buffer, coords_buffer.data(), coords_buffer.size() * sizeof(double));
}

void transformToVector(const std::vector<double>& oldVec,
                       std::vector<gp_Vec>& newVec) {
    const auto size = oldVec.size() / 3;
    newVec.reserve(size);
    for (std::size_t i = 0; i < size; i++) {
        const auto actualIndex = i * 3;
        newVec.emplace_back(oldVec[actualIndex], oldVec[actualIndex + 1], oldVec[actualIndex + 2]);
    }
}