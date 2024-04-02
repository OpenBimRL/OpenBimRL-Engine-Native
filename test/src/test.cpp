#include "lib.h"
#include "utils.h"

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