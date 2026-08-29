#include <gtest/gtest.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

#include "openbimrl/geometry/service.hpp"
#include "openbimrl/model/session.hpp"
#include "test_util.hpp"

namespace {

using OpenBimRL::Native::Geometry::boundsOf;
using OpenBimRL::Native::Geometry::buildingBounds;
using OpenBimRL::Native::Model::ActiveSession;
using OpenBimRL::Native::Model::IfcEntityRef;
using OpenBimRL::Native::Model::IfcSession;
using OpenBimRL::Native::Model::getGuid;

constexpr const char* kCorrectIfc = "correct.ifc";
constexpr const char* kMinimalIfc = "pathfinding_minimal.ifc";
constexpr const char* kIc6Ifc = "2024-10-25_IC6_ASR_Journal_Paper.ifc";
constexpr const char* kIc6StartGuid = "0Q4YfcC0z0pe5NxBNGF$xb";

struct BoundsProbeResult {
    bool finished = false;
    int exitCode = -1;
};

BoundsProbeResult probeBoundsAfterLoad(IfcEntityRef entity, int timeoutSeconds) {
    const pid_t pid = fork();
    if (pid < 0) {
        return {};
    }
    if (pid == 0) {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            _Exit(10);
        }
        const auto bounds = boundsOf(*session, entity);
        _Exit(bounds.has_value() ? 0 : 11);
    }

    int status = 0;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(timeoutSeconds);
    while (std::chrono::steady_clock::now() < deadline) {
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) {
            const int code =
                WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            return {true, code};
        }
        if (waited < 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    return {};
}

BoundsProbeResult probeBoundsWithReload(const std::filesystem::path& ifcPath,
                                        const std::string& guid,
                                        int timeoutSeconds) {
    const pid_t pid = fork();
    if (pid < 0) {
        return {};
    }
    if (pid == 0) {
        OpenBimRL::Engine::Utils::setSilent(true);
        auto session = IfcSession::open(ifcPath.string());
        if (!session) {
            _Exit(20);
        }
        ActiveSession::set(std::move(session));
        auto* active = ActiveSession::get();
        const auto entity = active->instanceByGuid(guid);
        if (!entity) {
            _Exit(21);
        }
        const auto bounds = boundsOf(*active, entity);
        _Exit(bounds.has_value() ? 0 : 22);
    }

    int status = 0;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(timeoutSeconds);
    while (std::chrono::steady_clock::now() < deadline) {
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) {
            const int code =
                WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            return {true, code};
        }
        if (waited < 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    kill(pid, SIGKILL);
    waitpid(pid, &status, 0);
    return {};
}

std::vector<IfcEntityRef> instancesOfType(const char* ifcType) {
    auto* session = ActiveSession::get();
    if (session == nullptr) {
        return {};
    }
    return session->instancesOf(ifcType);
}

void expectAllBoundsPresent(const std::vector<IfcEntityRef>& entities) {
    for (const auto& entity : entities) {
        ASSERT_TRUE(entity);
        EXPECT_TRUE(boundsOf(*ActiveSession::get(), entity).has_value())
            << "Missing bounds for " << getGuid(entity);
    }
}

void expectAllBoundsWithinTimeout(const std::vector<IfcEntityRef>& entities,
                                  int timeoutSeconds,
                                  bool requireBounds = false) {
    std::size_t withBounds = 0;
    std::size_t withoutBounds = 0;
    for (const auto& entity : entities) {
        ASSERT_TRUE(entity);
        const auto guid = getGuid(entity);
        const auto probe = probeBoundsAfterLoad(entity, timeoutSeconds);
        ASSERT_TRUE(probe.finished)
            << "boundsOf hung (> " << timeoutSeconds << "s) for " << guid;
        if (probe.exitCode == 0) {
            ++withBounds;
        } else if (probe.exitCode == 11) {
            ++withoutBounds;
            if (requireBounds) {
                ADD_FAILURE() << "boundsOf returned null for " << guid;
            }
        } else {
            FAIL() << "boundsOf failed for " << guid << " (exit "
                   << probe.exitCode << ")";
        }
    }
    std::cout << "boundsOf completed for " << entities.size()
              << " elements (" << withBounds << " with bounds, "
              << withoutBounds << " without bounds)\n";
}

}  // namespace

TEST(BoundingBoxRegression, CorrectIfc_AllSpaceBoundingBoxes) {
    ASSERT_TRUE(loadIntegrationIfc(kCorrectIfc));
    expectAllBoundsPresent(instancesOfType("IfcSpace"));
}

TEST(BoundingBoxRegression, MinimalIfc_AllWallBoundingBoxes) {
    if (!integrationResourceExists(kMinimalIfc)) {
        GTEST_SKIP() << "Missing " << kMinimalIfc
                     << " — copy from OpenBimRL-Engine/src/test/resources/";
    }
    ASSERT_TRUE(loadIntegrationIfc(kMinimalIfc));
    expectAllBoundsPresent(instancesOfType("IfcWall"));
}

TEST(BoundingBoxRegression, Ic6JournalPaper_Loads) {
    if (!integrationResourceExists(kIc6Ifc)) {
        GTEST_SKIP() << "Missing " << kIc6Ifc
                     << " — copy from OpenBimRL-Engine/src/test/resources/";
    }
    ASSERT_TRUE(loadIntegrationIfc(kIc6Ifc));
}

TEST(BoundingBoxRegression, Ic6JournalPaper_BuildingBoundsWithinTimeout) {
    if (!integrationResourceExists(kIc6Ifc)) {
        GTEST_SKIP() << "Missing " << kIc6Ifc;
    }
    ASSERT_TRUE(loadIntegrationIfc(kIc6Ifc));

    const pid_t pid = fork();
    if (pid == 0) {
        auto* session = ActiveSession::get();
        const auto bounds = buildingBounds(*session);
        _Exit(bounds.has_value() ? 0 : 1);
    }

    int status = 0;
    bool finished = false;
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(30);
    while (std::chrono::steady_clock::now() < deadline) {
        const pid_t waited = waitpid(pid, &status, WNOHANG);
        if (waited == pid) {
            finished = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    if (!finished) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        FAIL() << "buildingBounds hung on " << kIc6Ifc << " (>30s)";
    }

    ASSERT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0)
        << "buildingBounds returned no bounds";
}

TEST(BoundingBoxRegression, Ic6JournalPaper_StartElementBoundingBoxWithinTimeout) {
    if (!integrationResourceExists(kIc6Ifc)) {
        GTEST_SKIP() << "Missing " << kIc6Ifc;
    }

    const auto probe = probeBoundsWithReload(integrationResourcePath(kIc6Ifc),
                                             kIc6StartGuid, 120);
    ASSERT_TRUE(probe.finished)
        << "boundsOf hung on start element " << kIc6StartGuid;
    EXPECT_EQ(probe.exitCode, 0)
        << "boundsOf failed for start element (exit " << probe.exitCode << ")";
}

TEST(BoundingBoxRegression, Ic6JournalPaper_AllWallAndColumnBoundingBoxes) {
    if (!integrationResourceExists(kIc6Ifc)) {
        GTEST_SKIP() << "Missing " << kIc6Ifc;
    }
    ASSERT_TRUE(loadIntegrationIfc(kIc6Ifc));

    std::vector<IfcEntityRef> elements;
    for (const char* ifcType : {"IfcWall", "IfcColumn"}) {
        const auto chunk = instancesOfType(ifcType);
        elements.insert(elements.end(), chunk.begin(), chunk.end());
    }
    ASSERT_FALSE(elements.empty()) << "Expected walls/columns in " << kIc6Ifc;

    expectAllBoundsWithinTimeout(elements, 60);
}

TEST(BoundingBoxRegression, CompareModels_WallsCompleteOnSmallModels) {
    for (const char* model : {kCorrectIfc, kMinimalIfc}) {
        if (!integrationResourceExists(model)) {
            continue;
        }
        SCOPED_TRACE(model);
        ASSERT_TRUE(loadIntegrationIfc(model));
        const auto walls = instancesOfType("IfcWall");
        if (walls.empty()) {
            continue;
        }
        expectAllBoundsPresent(walls);
    }
}
