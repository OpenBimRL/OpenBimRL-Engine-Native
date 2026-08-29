#include <gtest/gtest.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3_add2.h>

#include <filesystem>

#include "ifc_elements.h"
#include "openbimrl/compat.hpp"
#include "openbimrl/geometry/service.hpp"
#include "openbimrl/geometry/types.hpp"
#include "openbimrl/model/session.hpp"
#include "openbimrl/properties/data.hpp"
#include "openbimrl_c_api.h"

namespace {

using OpenBimRL::Native::Geometry::boundsOf;
using OpenBimRL::Native::Geometry::buildingBounds;
using OpenBimRL::Native::Geometry::ElementFrame;
using OpenBimRL::Native::Geometry::extractFrame;
using OpenBimRL::Native::Geometry::footprintPolygonXY;
using OpenBimRL::Native::Model::ActiveSession;
using OpenBimRL::Native::Model::IfcEntityRef;
using OpenBimRL::Native::Properties::snapshot;
using OpenBimRL::Native::Properties::toJson;

bool loadTestIfc(const std::string& fileName) {
    OpenBimRL::Engine::Utils::setSilent(true);
    return initIfc(
        std::filesystem::path(RESOURCES_DIR).append(fileName).c_str());
}

}  // namespace

TEST(IFC4, LoadFile) { ASSERT_TRUE(loadTestIfc("correct.ifc")); }

TEST(IFC4, DetermineIFCVersion) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    ASSERT_TRUE(OpenBimRL::Engine::Utils::isIFC4());
    ASSERT_FALSE(OpenBimRL::Engine::Utils::isIFC2x3());
    ASSERT_FALSE(OpenBimRL::Engine::Utils::isIFC4X3());
}

TEST(IFC4X3, LoadFile) { ASSERT_TRUE(loadTestIfc("rail_test.ifc")); }

TEST(IFC4X3, DetermineIFCVersion) {
    ASSERT_TRUE(loadTestIfc("rail_test.ifc"));
    ASSERT_TRUE(OpenBimRL::Engine::Utils::isIFC4X3());
    ASSERT_FALSE(OpenBimRL::Engine::Utils::isIFC4());
    ASSERT_FALSE(OpenBimRL::Engine::Utils::isIFC2x3());
}

TEST(IFC4X3, FilterIfcRail) {
    ASSERT_TRUE(loadTestIfc("rail_test.ifc"));
    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    ASSERT_NE(file, nullptr);

    const auto rails = file->instances_by_type("IfcRail");
    ASSERT_TRUE(rails);
    ASSERT_EQ(rails->size(), 2);

    for (const auto rail : (*rails)) {
        const auto guid = OpenBimRL::Engine::Utils::getGUID(rail);
        EXPECT_FALSE(guid.empty());
        const auto data = OpenBimRL::Engine::Utils::getData(rail);
        EXPECT_EQ(data.ifcClass, "IfcRail");
        EXPECT_EQ(data.GUID, guid);
    }
}

TEST(Functions, FilterByGUID) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);

    const auto guid = "3bnVDGnRyHxfLHBF1T2vCN";
    const auto entity = session->instanceByGuid(guid);
    ASSERT_TRUE(entity);
    const auto* asObject = static_cast<IfcUtil::IfcBaseClass*>(entity.ptr)
                               ->as<Ifc4::IfcObject>(true);
    ASSERT_NE(asObject, nullptr);
    EXPECT_EQ(asObject->GlobalId(), guid);
}

TEST(Functions, FilterByElement) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);

    for (std::string_view ifc4ElementClass : ifc4ElementClasses) {
        const auto instances = session->instancesOf(ifc4ElementClass);
        for (const auto& instance : instances) {
            const auto* element =
                static_cast<IfcUtil::IfcBaseClass*>(instance.ptr)
                    ->as<Ifc4::IfcElement>(true);
            ASSERT_NE(element, nullptr);
            EXPECT_EQ(element->declaration().name(), ifc4ElementClass);
        }
    }
}

TEST(Functions, GetBoundingBox) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);

    const auto spaces = session->instancesOf("IfcSpace");
    for (const auto& space : spaces) {
        EXPECT_TRUE(boundsOf(*session, space).has_value());
    }
}

TEST(Functions, CalculateBuildingBounds) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);
    EXPECT_TRUE(buildingBounds(*session).has_value());
}

TEST(Utils, GeometryPolygon) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);

    for (const auto& wall : session->instancesOf("IfcWall")) {
        const auto polygon = footprintPolygonXY(*session, wall);
        (void)polygon;
    }
}

TEST(IFC4X3, ElementFrameFromPlacement) {
    ASSERT_TRUE(loadTestIfc("rail_test.ifc"));
    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    ASSERT_NE(file, nullptr);

    const auto rails = file->instances_by_type("IfcRail");
    ASSERT_TRUE(rails);
    ASSERT_GE(rails->size(), 1);

    OpenBimRL::Engine::Utils::ElementFrame frame{};
    ASSERT_TRUE(OpenBimRL::Engine::Utils::getElementFrame((*rails)[0], frame));
    EXPECT_EQ(frame.source, OpenBimRL::Engine::Utils::FrameSource::PLACEMENT);
    EXPECT_NEAR(frame.point[0], 0.0, 1e-6);
    EXPECT_NEAR(frame.point[1], 0.0, 1e-6);
    EXPECT_NEAR(frame.point[2], 0.0, 1e-6);
    EXPECT_NEAR(frame.axisX[0], 1.0, 1e-6);
    EXPECT_NEAR(frame.axisZ[2], 1.0, 1e-6);
}

TEST(Serializer, Serialize) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);

    for (const auto& space : session->instancesOf("IfcSpace")) {
        const auto data = snapshot(*session, space);
        const auto json = toJson(data);
        EXPECT_FALSE(json.empty());
    }
}

TEST(Functions, FilterByElementUnknownTypeIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);
    EXPECT_NO_THROW({
        const auto instances = session->instancesOf("IfcDefinitelyNotAType");
        EXPECT_TRUE(instances.empty());
    });
}

TEST(Functions, FilterByElementEmptyResultIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);
    const auto instances = session->instancesOf("IfcRail");
    EXPECT_TRUE(instances.empty());
}

TEST(Serializer, SnapshotNullPointerIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    auto* session = ActiveSession::get();
    ASSERT_NE(session, nullptr);
    const auto data = snapshot(*session, IfcEntityRef{nullptr});
    EXPECT_NO_THROW({ (void)toJson(data); });
}

TEST(Utils, ElementFrameWithoutRepresentationIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    ASSERT_NE(file, nullptr);

    const auto projects = file->instances_by_type("IfcProject");
    ASSERT_TRUE(projects);
    ASSERT_GE(projects->size(), 1);

    ElementFrame frame{};
    EXPECT_NO_THROW({
        (void)extractFrame(IfcEntityRef{(*projects)[0]}, frame);
    });
}
