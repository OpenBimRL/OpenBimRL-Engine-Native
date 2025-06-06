cmake_minimum_required(VERSION 3.25)
project(OpenBIMRL_Native)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

message("CMake is set to build for: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
# set(CMAKE_CXX_FLAGS_RELEASE "-O0")

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
set(BUILD_SHARED_LIBS ON)

# configure IfcOpenShell
set(BUILD_GEOMSERVER OFF)
set(BUILD_IFCPYTHON OFF)
set(COLLADA_SUPPORT OFF)
set(GLTF_SUPPORT OFF)
set(IFCXML_SUPPORT OFF)
set(USD_SUPPORT OFF)
set(HDF5_SUPPORT ON)
set(BUILD_IFCGEOM ON)

set(SCHEMA_VERSIONS "2x3" "4")

find_library(GMP_LIBRARY_FILE NAMES gmp mpir REQUIRED)
message("found GMP at: ${GMP_LIBRARY_FILE}")
cmake_path(GET GMP_LIBRARY_FILE PARENT_PATH GMP_LIBRARY_DIR)
message("transformed GMP dir to: ${GMP_LIBRARY_DIR}")

set(MPFR_LIBRARY_DIR ${GMP_LIBRARY_DIR})

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
        ifcopenshell
        GIT_REPOSITORY https://github.com/IfcOpenShell/IfcOpenShell.git
        GIT_TAG 60a70f5236528eaad63a1bb9109044d04a108679
        SOURCE_SUBDIR cmake
)

find_package(HDF5 REQUIRED)

FetchContent_MakeAvailable(ifcopenshell)

add_library(OpenBIMRL_Native SHARED
        functions/calculate_building_bounds.cpp
        functions/filter_by_element.cpp
        functions/function.cpp
        functions/filter_by_guid.cpp
        functions/get_bounding_box.cpp
        utils/ifc_init.cpp
        utils/ifc_serializer.cpp
        utils/ifc_data.cpp
        utils/ifc_data_utils.h
        utils/ifc_data_getters.cpp
        utils/ifc_data_handlers.cpp
        utils/ifc_data_utils.cpp
        utils/ifc_geometry_polygon.cpp
        utils/guid.cpp
)

set_property(TARGET OpenBIMRL_Native PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(
        OpenBIMRL_Native
        PUBLIC TKernel
        PUBLIC TKMath
        PUBLIC TKBRep
        PUBLIC TKGeomBase
        PUBLIC TKGeomAlgo
        PUBLIC TKG3d
        PUBLIC TKG2d
        PUBLIC TKShHealing
        PUBLIC TKTopAlgo
        PUBLIC TKMesh
        PUBLIC TKPrim
        PUBLIC TKBool
        PUBLIC TKBO
        PUBLIC TKFillet
        PUBLIC TKXSBase
        PUBLIC TKOffset
        PUBLIC TKHLR
        PUBLIC nlohmann_json::nlohmann_json
        PUBLIC IfcParse
        PUBLIC IfcGeom
        PUBLIC IfcGeom_ifc4
        PUBLIC IfcGeom_ifc2x3
)

FetchContent_GetProperties(ifcopenshell)

include_directories("${ifcopenshell_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/include" ${HDF5_INCLUDE_DIRS} "/usr/include/opencascade" "/usr/include/oce")

# tests

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

add_definitions(-DRESOURCES_DIR="${CMAKE_SOURCE_DIR}/test/resources")

add_executable(
        OpenBIMRL_Native_Test
        test/src/test.cpp
        test/src/ifc_elements.h
)

target_link_libraries(
        OpenBIMRL_Native_Test PRIVATE
        OpenBIMRL_Native
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(OpenBIMRL_Native_Test)
