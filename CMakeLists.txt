cmake_minimum_required(VERSION 3.25)
project(OpenBIMRL_Native)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# configure IfcOpenShell
set(MINIMAL_BUILD ON)

FetchContent_Declare(
        ifcopenshell
        GIT_REPOSITORY https://github.com/IfcOpenShell/IfcOpenShell.git
        GIT_TAG cd5afacc979e6ffea2abd20e69e9e07b6fa90e2e
        SOURCE_SUBDIR  cmake
)

# set(OCC_INCLUDE_DIR "/usr/include/opencascade")


FetchContent_MakeAvailable(ifcopenshell)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_library(OpenBIMRL_Native SHARED
        utils/ifc_init.cpp
        utils/ifc_pointer_to_string.cpp
        functions/filter_by_element.cpp
        functions/function.cpp
        functions/filter_by_guid.cpp
)

set_property(TARGET OpenBIMRL_Native PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(
        OpenBIMRL_Native PRIVATE
        IfcParse
        nlohmann_json::nlohmann_json
)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
        target_link_libraries(
                OpenBIMRL_Native PRIVATE
                -static-libgcc
                -static-libstdc++
                -static
        )
        target_compile_options(OpenBIMRL_Native PRIVATE
                -fexceptions
                -Wall
                -Werror
                -O3
        )
else ()
        target_compile_options(OpenBIMRL_Native PRIVATE
                -fexceptions
                -Wall
                -Werror
        )
endif()


FetchContent_GetProperties(ifcopenshell)

include_directories("${ifcopenshell_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/include")

# tests

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

add_definitions( -DRESOURCES_DIR="${CMAKE_SOURCE_DIR}/test/resources" )

add_executable(
        OpenBIMRL_Native_Test
        test/src/test.cpp
)
target_link_libraries(
        OpenBIMRL_Native_Test
        OpenBIMRL_Native
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(OpenBIMRL_Native_Test)
