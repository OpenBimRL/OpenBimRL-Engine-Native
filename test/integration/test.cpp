#include <gtest/gtest.h>
#include <ifcparse/Ifc4.h>
#include <ifcparse/Ifc4x3_add2.h>

#include <filesystem>

#include "ifc_elements.h"
#include "openbimrl/compat.hpp"
#include "openbimrl/ffi/rule_context.hpp"
#include "openbimrl_c_api.h"

namespace {

bool loadTestIfc(const std::string& fileName) {
    OpenBimRL::Engine::Utils::setSilent(true);
    return initIfc(
        std::filesystem::path(RESOURCES_DIR).append(fileName).c_str());
}

OpenBimRL::Native::Ffi::RuleContext& ctx() {
    return OpenBimRL::Native::Ffi::RuleContext::current();
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

    const auto guid = "3bnVDGnRyHxfLHBF1T2vCN";
    const auto getGUID = [=](uint32_t) { return guid; };
    const auto setPointer = [=](uint32_t index, void* result) {
        if (index != 0) FAIL() << "filterByGUID returned nothing on output 0";
        if (!result) FAIL() << "filterByGUID returned null pointer!";
        try {
            const auto ifcItem =
                ((IfcUtil::IfcBaseClass*)(result))->as<Ifc4::IfcObject>(true);
            const auto itemGUID = ifcItem->GlobalId();
            EXPECT_EQ(itemGUID.compare(guid), 0)
                << "GUIDs: [" + itemGUID + ", " + guid + "] do not match!";
        } catch (IfcParse::IfcException& e) {
            FAIL() << e.what();
        }
    };
    ctx().getInputPointer = nullptr;
    ctx().getInputDouble = nullptr;
    ctx().getInputInt = nullptr;
    ctx().getInputString = std::function(getGUID);
    ctx().setOutputPointer = std::function(setPointer);
    ctx().setOutputDouble = nullptr;
    ctx().setOutputInt = nullptr;
    ctx().setOutputString = nullptr;
    ctx().setOutputArray = nullptr;

    filterByGUID();
}

TEST(Functions, FilterByElement) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));

    for (std::string_view ifc4ElementClass : ifc4ElementClasses) {
        const auto getType = [=](uint32_t) { return ifc4ElementClass.data(); };
        void* buffer = nullptr;
        std::size_t elements_buffer_size = 0;
        const auto setOutputArray = [&buffer, &elements_buffer_size](
                                        uint32_t, std::size_t size) {
            buffer = calloc(size, 1);
            elements_buffer_size = size;
            return buffer;
        };

        ctx().getInputPointer = nullptr;
        ctx().getInputDouble = nullptr;
        ctx().getInputInt = nullptr;
        ctx().getInputString = std::function(getType);
        ctx().setOutputPointer = nullptr;
        ctx().setOutputDouble = nullptr;
        ctx().setOutputInt = nullptr;
        ctx().setOutputString = nullptr;
        ctx().setOutputArray = std::function(setOutputArray);

        filterByElement();

        const auto elementArray = (IfcUtil::IfcBaseClass**)buffer;
        const auto elements = elements_buffer_size / sizeof(void*);

        for (std::size_t i = 0; i < elements; ++i) {
            try {
                const auto element =
                    elementArray[i]->as<Ifc4::IfcElement>(true);
                const auto className = element->declaration().name();
                EXPECT_EQ(className, ifc4ElementClass);
            } catch (IfcParse::IfcException& e) {
                FAIL() << "While parsing this error occurred: " << e.what();
            }
        }
        if (buffer) free(buffer);
    }
}

TEST(Functions, GetBoundingBox) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));

    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    std::uint32_t counter = 0;
    std::vector<void*> allocations;

    ctx().getInputPointer = std::function([file, &counter](uint32_t) {
        return (void*)(*(file->instances_by_type("IfcSpace")->begin() +
                         counter));
    });
    ctx().getInputDouble = nullptr;
    ctx().getInputInt = nullptr;
    ctx().getInputString = nullptr;
    ctx().setOutputArray = [&allocations](uint32_t, std::size_t size) {
        void* buffer = calloc(size, 1);
        allocations.push_back(buffer);
        return buffer;
    };

    for (; counter < file->instances_by_type("IfcSpace")->size(); counter++)
        getBoundingBox();

    for (void* p : allocations) free(p);
}

TEST(Functions, CalculateBuildingBounds) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));

    void* buffer = nullptr;
    ctx().setOutputArray = [&buffer](uint32_t, std::size_t size) {
        buffer = calloc(size, 1);
        return buffer;
    };

    calculatingBuildingBounds();
    free(buffer);
}

TEST(Utils, GeometryPolygon) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));

    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    boost::shared_ptr<aggregate_of_instance> ptr;
    try {
        ptr = file->instances_by_type("IfcWall");
    } catch (const IfcParse::IfcException&) {
    }

    for (const auto item : (*ptr)) {
        const auto size = request_geometry_polygon(item);
        if (!size) continue;
        auto* str = (char*)std::calloc(size + 1, sizeof(double));
        copy_geometry_polygon(str);
        std::free(str);
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

    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    boost::shared_ptr<aggregate_of_instance> ptr;
    try {
        ptr = file->instances_by_type("IfcSpace");
    } catch (const IfcParse::IfcException&) {
    }

    for (const auto item : (*ptr)) {
        const auto size = request_ifc_object_json_size(item);
        auto* str = (char*)std::calloc(size + 1, 1);
        ifc_object_to_json(str);
        std::free(str);
    }
}

// Native issue #3 / #4: unknown or absent types must not throw / segfault.
TEST(Functions, FilterByElementUnknownTypeIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));

    bool sawNullPointer = false;
    bool sawEmptyArray = false;
    ctx().getInputString = [](uint32_t) { return "IfcDefinitelyNotAType"; };
    ctx().setOutputPointer = [&](uint32_t, void* p) {
        if (p == nullptr) sawNullPointer = true;
    };
    ctx().setOutputArray = [&](uint32_t, std::size_t size) -> void* {
        if (size == 0) sawEmptyArray = true;
        return nullptr;
    };

    EXPECT_NO_THROW(filterByElement());
    EXPECT_TRUE(sawNullPointer || sawEmptyArray);
}

TEST(Functions, FilterByElementEmptyResultIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));

    bool sawEmpty = false;
    ctx().getInputString = [](uint32_t) { return "IfcRail"; };  // IFC4X3-only
    ctx().setOutputPointer = [&](uint32_t, void* p) {
        if (p == nullptr) sawEmpty = true;
    };
    ctx().setOutputArray = [&](uint32_t, std::size_t size) -> void* {
        if (size == 0) sawEmpty = true;
        return nullptr;
    };

    EXPECT_NO_THROW(filterByElement());
    EXPECT_TRUE(sawEmpty);
}

// Native issue #5: property snapshot must tolerate null / non-product entities.
TEST(Serializer, SnapshotNullPointerIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    const auto size = request_ifc_object_json_size(nullptr);
    ASSERT_GT(size, 0u);
    auto* str = (char*)std::calloc(size + 1, 1);
    ifc_object_to_json(str);
    EXPECT_NE(str[0], '\0');
    std::free(str);
}

// Native issue #6: entities without Representation must not segfault.
TEST(Utils, ElementFrameWithoutRepresentationIsSafe) {
    ASSERT_TRUE(loadTestIfc("correct.ifc"));
    IfcParse::IfcFile* file = OpenBimRL::Engine::Utils::getCurrentFile();
    ASSERT_NE(file, nullptr);

    const auto projects = file->instances_by_type("IfcProject");
    ASSERT_TRUE(projects);
    ASSERT_GE(projects->size(), 1);

    OpenBimRL::Engine::Utils::ElementFrame frame{};
    // May fail to extract a frame, but must not abort.
    EXPECT_NO_THROW({
        (void)OpenBimRL::Engine::Utils::getElementFrame((*projects)[0], frame);
    });
}
