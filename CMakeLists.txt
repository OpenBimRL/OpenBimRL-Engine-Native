cmake_minimum_required(VERSION 3.28)
project(OpenBIMRL_Native)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FetchContent_Declare(
        ifcopenshell
        GIT_REPOSITORY https://github.com/IfcOpenShell/IfcOpenShell.git
        GIT_TAG cd5afacc979e6ffea2abd20e69e9e07b6fa90e2e
        SOURCE_SUBDIR  cmake
)

set(MINIMAL_BUILD ON)
# set(OCC_INCLUDE_DIR "/usr/include/opencascade")

FetchContent_MakeAvailable(ifcopenshell)

add_library(OpenBIMRL_Native STATIC library.cpp)

target_link_libraries(
        OpenBIMRL_Native
        PRIVATE IfcParse
)

FetchContent_GetProperties(ifcopenshell)

include_directories("${ifcopenshell_SOURCE_DIR}/src")

# tests

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

add_executable(
        OpenBIMRL_Native_Test
        test/test.cpp
)
target_link_libraries(
        OpenBIMRL_Native_Test
        OpenBIMRL_Native
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(OpenBIMRL_Native_Test)
