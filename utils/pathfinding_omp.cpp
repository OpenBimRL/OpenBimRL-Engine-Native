#include <omp.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <unordered_set>
#include <vector>

#include "lib.h"
#include "utils.h"
#include "ifc_geom_utils.h"

namespace {
using namespace OpenBimRL::Engine;

struct Segment2D {
    double x1;
    double y1;
    double x2;
    double y2;
};

constexpr double EPS = 1e-9;

double orientation(double ax, double ay, double bx, double by, double cx,
                   double cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

bool on_segment(double ax, double ay, double bx, double by, double px,
                double py) {
    return px >= std::min(ax, bx) - EPS && px <= std::max(ax, bx) + EPS &&
           py >= std::min(ay, by) - EPS && py <= std::max(ay, by) + EPS;
}

bool segments_intersect(const Segment2D& a, const Segment2D& b) {
    const auto o1 = orientation(a.x1, a.y1, a.x2, a.y2, b.x1, b.y1);
    const auto o2 = orientation(a.x1, a.y1, a.x2, a.y2, b.x2, b.y2);
    const auto o3 = orientation(b.x1, b.y1, b.x2, b.y2, a.x1, a.y1);
    const auto o4 = orientation(b.x1, b.y1, b.x2, b.y2, a.x2, a.y2);

    const auto sgn = [](double v) -> int {
        if (v > EPS) return 1;
        if (v < -EPS) return -1;
        return 0;
    };

    const auto s1 = sgn(o1);
    const auto s2 = sgn(o2);
    const auto s3 = sgn(o3);
    const auto s4 = sgn(o4);

    if (s1 != s2 && s3 != s4) return true;
    if (s1 == 0 && on_segment(a.x1, a.y1, a.x2, a.y2, b.x1, b.y1)) return true;
    if (s2 == 0 && on_segment(a.x1, a.y1, a.x2, a.y2, b.x2, b.y2)) return true;
    if (s3 == 0 && on_segment(b.x1, b.y1, b.x2, b.y2, a.x1, a.y1)) return true;
    if (s4 == 0 && on_segment(b.x1, b.y1, b.x2, b.y2, a.x2, a.y2)) return true;
    return false;
}

std::vector<Segment2D> extract_segments_from_pointer(JNA::Pointer ifc_pointer) {
    std::vector<Segment2D> segments;

    const auto guid =
        Utils::getGUID(static_cast<Types::IFC::IfcObjectPointer>(ifc_pointer));

    std::vector<IfcGeom::filter_t> filters;
    filters.emplace_back(std::function(([guid](IfcUtil::IfcBaseEntity* entity) {
        return guid == Utils::getGUID(entity);
    })));

    auto geom_iterator = Utils::createGeometryIterator(Utils::getCurrentFile(), filters);
    if (!geom_iterator->initialize()) return segments;

    IfcGeom::Element* geom_object = nullptr;
    do {
        geom_object = geom_iterator->get();
        if (geom_object) break;
    } while (geom_iterator->next());
    if (!geom_object) return segments;

    const auto* element =
        static_cast<const IfcGeom::TriangulationElement*>(geom_object);
    const IfcGeom::Representation::Triangulation& mesh = element->geometry();

    const auto& vertices_raw = mesh.verts();
    const auto vertices_count = vertices_raw.size() / 3;
    std::vector<std::array<double, 2>> vertices_xy;
    vertices_xy.reserve(vertices_count);
    for (std::size_t i = 0; i < vertices_count; i++) {
        const auto idx = i * 3;
        vertices_xy.push_back({vertices_raw[idx], vertices_raw[idx + 1]});
    }

    const auto& faces_raw = mesh.faces();
    const auto faces_count = faces_raw.size() / 3;
    segments.reserve(faces_count * 3);
    for (std::size_t i = 0; i < faces_count; ++i) {
        const auto idx = i * 3;
        const auto a = static_cast<std::size_t>(faces_raw[idx]);
        const auto b = static_cast<std::size_t>(faces_raw[idx + 1]);
        const auto c = static_cast<std::size_t>(faces_raw[idx + 2]);
        if (a >= vertices_xy.size() || b >= vertices_xy.size() ||
            c >= vertices_xy.size())
            continue;
        const auto va = vertices_xy[a];
        const auto vb = vertices_xy[b];
        const auto vc = vertices_xy[c];
        segments.push_back({va[0], va[1], vb[0], vb[1]});
        segments.push_back({vb[0], vb[1], vc[0], vc[1]});
        segments.push_back({vc[0], vc[1], va[0], va[1]});
    }

    return segments;
}

std::vector<Segment2D> collect_segments(const JNA::Pointer* pointers,
                                        uint32_t pointer_count) {
    std::vector<Segment2D> out;
    for (uint32_t i = 0; i < pointer_count; ++i) {
        if (pointers[i] == nullptr) continue;
        auto local = extract_segments_from_pointer(pointers[i]);
        out.insert(out.end(), local.begin(), local.end());
    }
    return out;
}

bool intersects_any(const Segment2D& line, const std::vector<Segment2D>& segs) {
    for (const auto& seg : segs) {
        if (segments_intersect(line, seg)) return true;
    }
    return false;
}
}  // namespace

extern "C" void calculate_path_edge_costs(
    const double* points_xy, uint32_t point_count,
    const uint32_t* edge_point_indices, uint32_t edge_count,
    const JNA::Pointer* passage_pointers, uint32_t passage_count,
    const JNA::Pointer* obstacle_pointers, uint32_t obstacle_count,
    double* output_costs) {
    if (!points_xy || !edge_point_indices || !output_costs) return;

    const auto passage_segments = collect_segments(passage_pointers, passage_count);
    const auto obstacle_segments =
        collect_segments(obstacle_pointers, obstacle_count);

#pragma omp parallel for if (edge_count > 256)
    for (uint32_t i = 0; i < edge_count; ++i) {
        const auto a_idx = edge_point_indices[i * 2];
        const auto b_idx = edge_point_indices[i * 2 + 1];
        if (a_idx >= point_count || b_idx >= point_count) {
            output_costs[i] = std::numeric_limits<double>::infinity();
            continue;
        }
        const auto ax = points_xy[a_idx * 2];
        const auto ay = points_xy[a_idx * 2 + 1];
        const auto bx = points_xy[b_idx * 2];
        const auto by = points_xy[b_idx * 2 + 1];
        const Segment2D line{ax, ay, bx, by};

        const auto dx = ax - bx;
        const auto dy = ay - by;
        const auto euclidean = std::sqrt(dx * dx + dy * dy);

        if (intersects_any(line, passage_segments)) {
            output_costs[i] = euclidean;
            continue;
        }
        if (intersects_any(line, obstacle_segments)) {
            output_costs[i] = std::numeric_limits<double>::infinity();
            continue;
        }
        output_costs[i] = euclidean;
    }
}
