#include "openbimrl_c_api.h"

#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "openbimrl/ffi/rule_context.hpp"
#include "openbimrl/geometry/service.hpp"
#include "openbimrl/geometry/types.hpp"
#include "openbimrl/model/entity.hpp"
#include "openbimrl/model/session.hpp"
#include "openbimrl/pathfinding/edge_costs.hpp"
#include "openbimrl/properties/data.hpp"

using OpenBimRL::Native::Ffi::RuleContext;
using OpenBimRL::Native::Geometry::boundsToXzyArray;
using OpenBimRL::Native::Geometry::buildingBounds;
using OpenBimRL::Native::Geometry::boundsOf;
using OpenBimRL::Native::Geometry::ElementFrame;
using OpenBimRL::Native::Geometry::extractFrame;
using OpenBimRL::Native::Geometry::extractSegmentsXY;
using OpenBimRL::Native::Geometry::footprintPolygonXY;
using OpenBimRL::Native::Model::ActiveSession;
using OpenBimRL::Native::Model::IfcEntityRef;
using OpenBimRL::Native::Pathfinding::computeEdgeCosts;
using OpenBimRL::Native::Pathfinding::Edge;
using OpenBimRL::Native::Pathfinding::Point2;
using OpenBimRL::Native::Pathfinding::Segment2;
using OpenBimRL::Native::Properties::snapshot;
using OpenBimRL::Native::Properties::toJson;

namespace {

thread_local std::vector<double> g_polygonBuffer;
thread_local std::string g_jsonBuffer;

IfcEntityRef entityFromPointer(void* ptr) { return IfcEntityRef{ptr}; }

}  // namespace

extern "C" void filterByElement(void) {
    auto& ctx = RuleContext::current();
    auto* session = ActiveSession::get();
    if (!session || !ctx.getInputString || !ctx.setOutputPointer) {
        if (ctx.setOutputPointer) ctx.setOutputPointer(0, nullptr);
        return;
    }

    const char* input = ctx.getInputString(0);
    if (!input) {
        ctx.setOutputPointer(0, nullptr);
        return;
    }

    const auto instances = session->instancesOf(input);
    if (instances.empty()) {
        // Empty result is valid: notify both callback styles used by JVM/tests.
        if (ctx.setOutputArray) ctx.setOutputArray(0, 0);
        if (ctx.setOutputPointer) ctx.setOutputPointer(0, nullptr);
        return;
    }

    if (!ctx.setOutputArray) {
        if (ctx.setOutputPointer) ctx.setOutputPointer(0, instances.front().ptr);
        return;
    }
    auto* buffer = static_cast<void**>(
        ctx.setOutputArray(0, instances.size() * sizeof(void*)));
    if (!buffer) return;
    for (std::size_t i = 0; i < instances.size(); ++i) {
        buffer[i] = instances[i].ptr;
    }
}

extern "C" void filterByGUID(void) {
    auto& ctx = RuleContext::current();
    auto* session = ActiveSession::get();
    void* ptr = nullptr;
    if (session && ctx.getInputString) {
        const char* input = ctx.getInputString(0);
        if (input) {
            ptr = session->instanceByGuid(input).ptr;
        }
    }
    if (ctx.setOutputPointer) ctx.setOutputPointer(0, ptr);
}

extern "C" void getBoundingBox(void) {
    auto& ctx = RuleContext::current();
    auto* session = ActiveSession::get();
    if (!session || !ctx.getInputPointer || !ctx.setOutputArray) return;

    const auto entity = entityFromPointer(ctx.getInputPointer(0));
    if (!entity) return;

    const auto bounds = boundsOf(*session, entity);
    if (!bounds) return;

    auto* buffer = ctx.setOutputArray(0, 6 * sizeof(double));
    double values[6];
    boundsToXzyArray(*bounds, values);
    std::memcpy(buffer, values, sizeof(values));
}

extern "C" void getElementFrame(void) {
    auto& ctx = RuleContext::current();
    if (!ctx.getInputPointer || !ctx.setOutputArray) return;

    const auto entity = entityFromPointer(ctx.getInputPointer(0));
    ElementFrame frame{};
    if (!extractFrame(entity, frame)) return;

    auto* buffer = ctx.setOutputArray(0, 10 * sizeof(double));
    std::memcpy(buffer, frame.point, 3 * sizeof(double));
    std::memcpy(static_cast<char*>(buffer) + 3 * sizeof(double), frame.axisX,
                3 * sizeof(double));
    std::memcpy(static_cast<char*>(buffer) + 6 * sizeof(double), frame.axisZ,
                3 * sizeof(double));
    if (ctx.setOutputInt) {
        ctx.setOutputInt(1, static_cast<uint32_t>(frame.source));
    }
}

extern "C" void calculatingBuildingBounds(void) {
    auto& ctx = RuleContext::current();
    auto* session = ActiveSession::get();
    if (!session || !ctx.setOutputArray) return;

    const auto bounds = buildingBounds(
        *session, static_cast<int>(std::thread::hardware_concurrency()));
    if (!bounds) return;

    auto* buffer = ctx.setOutputArray(0, 6 * sizeof(double));
    double values[6];
    boundsToXzyArray(*bounds, values);
    std::memcpy(buffer, values, sizeof(values));
}

extern "C" size_t request_geometry_polygon(void* ifc_pointer) {
    g_polygonBuffer.clear();
    auto* session = ActiveSession::get();
    if (!session || !ifc_pointer) return 0;
    g_polygonBuffer =
        footprintPolygonXY(*session, entityFromPointer(ifc_pointer));
    return g_polygonBuffer.size();
}

extern "C" void copy_geometry_polygon(void* buffer) {
    if (!buffer || g_polygonBuffer.empty()) return;
    std::memcpy(buffer, g_polygonBuffer.data(),
                g_polygonBuffer.size() * sizeof(double));
}

extern "C" size_t request_ifc_object_json_size(void* ifc_pointer) {
    g_jsonBuffer.clear();
    auto* session = ActiveSession::get();
    if (!session || !ifc_pointer) {
        g_jsonBuffer = "{}";
        return g_jsonBuffer.size() + 1;
    }
    try {
        const auto data = snapshot(*session, entityFromPointer(ifc_pointer));
        g_jsonBuffer = toJson(data);
    } catch (...) {
        g_jsonBuffer = "{}";
    }
    return g_jsonBuffer.size() + 1;
}

extern "C" void ifc_object_to_json(void* buffer) {
    if (!buffer) return;
    std::memcpy(buffer, g_jsonBuffer.c_str(), g_jsonBuffer.size() + 1);
}

extern "C" void calculate_path_edge_costs(
    const double* points_xy, uint32_t point_count,
    const uint32_t* edge_point_indices, uint32_t edge_count,
    const void* const* passage_pointers, uint32_t passage_count,
    const void* const* obstacle_pointers, uint32_t obstacle_count,
    double* output_costs) {
    if (!points_xy || !edge_point_indices || !output_costs) return;

    auto* session = ActiveSession::get();
    std::vector<Segment2> passages;
    std::vector<Segment2> obstacles;
    if (session) {
        for (uint32_t i = 0; i < passage_count; ++i) {
            if (!passage_pointers || !passage_pointers[i]) continue;
            auto local =
                extractSegmentsXY(*session, entityFromPointer(
                                                const_cast<void*>(passage_pointers[i])));
            passages.insert(passages.end(), local.begin(), local.end());
        }
        for (uint32_t i = 0; i < obstacle_count; ++i) {
            if (!obstacle_pointers || !obstacle_pointers[i]) continue;
            auto local =
                extractSegmentsXY(*session, entityFromPointer(
                                                const_cast<void*>(obstacle_pointers[i])));
            obstacles.insert(obstacles.end(), local.begin(), local.end());
        }
    }

    std::vector<Point2> points(point_count);
    for (uint32_t i = 0; i < point_count; ++i) {
        points[i] = {points_xy[i * 2], points_xy[i * 2 + 1]};
    }
    std::vector<Edge> edges(edge_count);
    for (uint32_t i = 0; i < edge_count; ++i) {
        edges[i] = {edge_point_indices[i * 2], edge_point_indices[i * 2 + 1]};
    }
    std::vector<double> costs(edge_count);
    computeEdgeCosts(points, edges, passages, obstacles, costs);
    std::memcpy(output_costs, costs.data(), edge_count * sizeof(double));
}
