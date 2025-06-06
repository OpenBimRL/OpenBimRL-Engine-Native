#include <gtest/gtest.h>
#include <ifcparse/Ifc4.h>

#include <filesystem>

#include "functions.h"
#include "ifc_elements.h"
#include "lib.h"
#include "utils.h"

TEST(IFC4, LoadFile) {
    OpenBimRL::Engine::Utils::setSilent(true);

    const auto init =
        initIfc(std::filesystem::path(RESOURCES_DIR).append("correct.ifc").c_str());

    ASSERT_TRUE(init);
}

TEST(IFC4, DetermineIFCVersion) {
    ASSERT_TRUE(OpenBimRL::Engine::Utils::isIFC4());
    ASSERT_FALSE(OpenBimRL::Engine::Utils::isIFC2x3());
}

TEST(Functions, FilterByGUID) {
    // IfcOpeningElement#3bnVDGnRyHxfLHBF1T2vCN
    const auto guid = "3bnVDGnRyHxfLHBF1T2vCN";
    const auto getGUID = [=](uint32_t) { return guid; };
    const auto setPointer = [=](uint32_t index, void *result) {
        if (index != 0) FAIL() << "filterByGUID returned nothing on output 0";
        if (!result) FAIL() << "filterByGUID returned null pointer!";
        try {
            const auto ifcItem =
                ((IfcUtil::IfcBaseClass *)(result))->as<Ifc4::IfcObject>(true);
            const auto itemGUID = ifcItem->GlobalId();
            EXPECT_EQ(itemGUID.compare(guid), 0)
                << "GUIDs: [" + itemGUID + ", " + guid + "] do not match!";
        } catch (IfcParse::IfcException &e) {
            FAIL() << e.what();
        }
    };
    OpenBimRL::Engine::Functions::getInputPointer = nullptr;
    OpenBimRL::Engine::Functions::getInputDouble = nullptr;
    OpenBimRL::Engine::Functions::getInputInt = nullptr;
    OpenBimRL::Engine::Functions::getInputString = std::function(getGUID);

    OpenBimRL::Engine::Functions::setOutputPointer = std::function(setPointer);
    OpenBimRL::Engine::Functions::setOutputDouble = nullptr;
    OpenBimRL::Engine::Functions::setOutputInt = nullptr;
    OpenBimRL::Engine::Functions::setOutputString = nullptr;
    OpenBimRL::Engine::Functions::setOutputArray = nullptr;

    filterByGUID();
}

TEST(Functions, FilterByElement) {
    for (std::string_view ifc4ElementClass : ifc4ElementClasses) {
        const auto getType = [=](uint32_t) { return ifc4ElementClass.data(); };
        const auto setPointer = [&ifc4ElementClass](uint32_t, void *) {
            std::cout
                << "No elements of type "
                << ifc4ElementClass
                << " found"
                << std::endl;
        };
        void *buffer = nullptr;
        std::size_t elements_buffer_size = 0;
        const auto setOutputArray = [&buffer, &elements_buffer_size](
                                        uint32_t position, std::size_t size) {
            buffer = calloc(size, 1);
            elements_buffer_size = size;
            return buffer;
        };

        OpenBimRL::Engine::Functions::getInputPointer = nullptr;
        OpenBimRL::Engine::Functions::getInputDouble = nullptr;
        OpenBimRL::Engine::Functions::getInputInt = nullptr;
        OpenBimRL::Engine::Functions::getInputString = std::function(getType);

        OpenBimRL::Engine::Functions::setOutputPointer =
            std::function(setPointer);
        OpenBimRL::Engine::Functions::setOutputDouble = nullptr;
        OpenBimRL::Engine::Functions::setOutputInt = nullptr;
        OpenBimRL::Engine::Functions::setOutputString = nullptr;
        OpenBimRL::Engine::Functions::setOutputArray =
            std::function(setOutputArray);

        filterByElement();

        const auto elementArray = (IfcUtil::IfcBaseClass **)buffer;

        const auto elements = elements_buffer_size / sizeof(void *);

        for (std::size_t i = 0; i < elements; ++i) {
            try {
                const auto element =
                    elementArray[i]->as<Ifc4::IfcElement>(true);
                const auto className = element->data().type()->name();
                if (className != ifc4ElementClass)
                    std::cout << "Element is not of type " << ifc4ElementClass
                           << ". Instead it is an: " << className;
            } catch (IfcParse::IfcException &e) {
                FAIL() << "While parsing this error occurred: " << e.what();
            }
        }
        if (buffer) free(buffer);
    }
}

TEST(Functions, GetBoundingBox) {
    IfcParse::IfcFile *file =
        OpenBimRL::Engine::Utils::getCurrentFile();  // get active file
    std::uint32_t counter = 0;

    OpenBimRL::Engine::Functions::getInputPointer =
        std::function([file, &counter](uint32_t) {
            return (void *)(*(file->instances_by_type("IfcSpace")->begin() +
                              counter));
        });
    OpenBimRL::Engine::Functions::getInputDouble = nullptr;
    OpenBimRL::Engine::Functions::getInputInt = nullptr;
    OpenBimRL::Engine::Functions::getInputString = nullptr;

    for (counter; counter < file->instances_by_type("IfcSpace")->size();
         counter++)
        getBoundingBox();
}

TEST(Functions, CalculateBuildingBounds) {
    void *buffer;
    std::size_t elements_buffer_size = 0;
    const auto setOutputArray = [&buffer, &elements_buffer_size](
                                    uint32_t position, std::size_t size) {
        buffer = calloc(size, 1);
        elements_buffer_size = size;
        return buffer;
    };

    OpenBimRL::Engine::Functions::setOutputArray =
        std::function(setOutputArray);

    calculatingBuildingBounds();

    free(buffer);
}

TEST(Utils, GeometryPolygon) {
    const auto type = "IfcWall";
    IfcParse::IfcFile *file =
        OpenBimRL::Engine::Utils::getCurrentFile();  // get active file
    boost::shared_ptr<aggregate_of_instance> ptr;
    try {
        ptr = file->instances_by_type(type);  // find ifc obj by guid
    }
    // thank you IfcOpenShell for documenting that this error exists...
    catch (const IfcParse::IfcException &) {
    }

    for (const auto item : (*ptr)) {
        const auto size = request_geometry_polygon(item);
        if (!size) continue;
        auto str = (char *)std::calloc(size + 1, sizeof(double));
        copy_geometry_polygon(str);
        std::cout << std::string(str) << std::endl;
        std::free(str);
    }
}

TEST(Serializer, Serialize) {
    const auto type = "IfcSpace";
    IfcParse::IfcFile *file =
        OpenBimRL::Engine::Utils::getCurrentFile();  // get active file
    boost::shared_ptr<aggregate_of_instance> ptr;
    try {
        ptr = file->instances_by_type(type);  // find ifc obj by guid
    }
    // thank you IfcOpenShell for documenting that this error exists...
    catch (const IfcParse::IfcException &) {
    }

    for (const auto item : (*ptr)) {
        const auto size = request_ifc_object_json_size(item);
        auto str = (char *)std::calloc(size + 1, 1);
        ifc_object_to_json(str);
        // std::cout << std::string(str) << std::endl;
        std::free(str);
    }
}