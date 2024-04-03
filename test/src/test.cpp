#include "lib.h"
#include "utils.h"
#include "functions.h"

#include <ifcparse/Ifc4.h>
#include <filesystem>
#include <gtest/gtest.h>

TEST(IFC4, LoadFile) {
    OpenBimRL::Engine::Utils::setSilent(true);

    const auto init = initIfc(std::filesystem::path(RESOURCES_DIR).append("test.ifc").c_str());

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
    const auto setPointer = [=](uint32_t index, void* result) {
        if (index != 0)
            FAIL() << "filterByGUID returned nothing on output 0";
        if (!result)
            FAIL() << "filterByGUID returned null pointer!";
        try {
            const auto ifcItem = dynamic_cast<Ifc4::IfcObject *>((IfcUtil::IfcBaseClass *)(result));
            const auto itemGUID = ifcItem->GlobalId();
            EXPECT_EQ(itemGUID.compare(guid), 0) << "GUIDs: [" + itemGUID + ", " + guid + "] do not match!";
        }
        catch (std::exception &e) {
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
    const auto type = "IfcSpace";
    const auto getType = [=](uint32_t) { return type; };
    const auto setPointer = [](uint32_t, void *) { FAIL() << "returned null pointer!"; };
    void *buffer;
    std::size_t elements = 0;
    const auto setOutputArray = [&buffer, &elements](uint32_t position, std::size_t size) {
        buffer = calloc(size, sizeof(void *));
        elements = size;
        return buffer;
    };

    OpenBimRL::Engine::Functions::getInputPointer = nullptr;
    OpenBimRL::Engine::Functions::getInputDouble = nullptr;
    OpenBimRL::Engine::Functions::getInputInt = nullptr;
    OpenBimRL::Engine::Functions::getInputString = std::function(getType);

    OpenBimRL::Engine::Functions::setOutputPointer = std::function(setPointer);
    OpenBimRL::Engine::Functions::setOutputDouble = nullptr;
    OpenBimRL::Engine::Functions::setOutputInt = nullptr;
    OpenBimRL::Engine::Functions::setOutputString = nullptr;
    OpenBimRL::Engine::Functions::setOutputArray = std::function(setOutputArray);

    filterByElement();


    if (!buffer) {
        FAIL();
    }

    const auto elementArray = (IfcUtil::IfcBaseClass **) buffer;

    ASSERT_EQ(elements, 326);

    for (std::size_t i = 0; i < elements; ++i) {
        try {
            const auto element = elementArray[i]->as<Ifc4::IfcSpace>(true);
            const auto className = element->data().type()->name();
            if (className != "IfcSpace")
                FAIL() << "Element is not of type IfcSpace. Instead it is an: " << className;
        } catch (IfcParse::IfcException &e) {
            FAIL() << "Element was not of type IfcSpace. While parsing this error occurred: " << e.what();
        }
    }

    free(buffer);
}