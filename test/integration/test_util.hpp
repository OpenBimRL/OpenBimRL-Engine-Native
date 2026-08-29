#pragma once

#include <filesystem>
#include <string>

#include "openbimrl/compat.hpp"
#include "openbimrl_c_api.h"

#ifndef RESOURCES_DIR
#error "RESOURCES_DIR must be defined by the test target"
#endif

inline std::filesystem::path integrationResourcePath(const std::string& fileName) {
    return std::filesystem::path(RESOURCES_DIR) / fileName;
}

inline bool integrationResourceExists(const std::string& fileName) {
    return std::filesystem::exists(integrationResourcePath(fileName));
}

inline bool loadIntegrationIfc(const std::string& fileName) {
    OpenBimRL::Engine::Utils::setSilent(true);
    return initIfc(integrationResourcePath(fileName).c_str());
}
