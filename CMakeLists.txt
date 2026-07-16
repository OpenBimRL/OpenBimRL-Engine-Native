cmake_minimum_required(VERSION 3.25)
project(OpenBIMRL_Native)

include(FetchContent)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

message("CMake is set to build for: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found: ${OpenMP_CXX_VERSION}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

option(OPENBIMRL_ENABLE_ROCM_OFFLOAD "Enable ROCm OpenMP GPU offloading" OFF)
set(OPENBIMRL_ROCM_OFFLOAD_ARCH "" CACHE STRING "ROCm offload architecture, e.g. gfx1100")

if(OPENBIMRL_ENABLE_ROCM_OFFLOAD)
    if(NOT OPENBIMRL_ROCM_OFFLOAD_ARCH)
        message(FATAL_ERROR "OPENBIMRL_ENABLE_ROCM_OFFLOAD=ON requires OPENBIMRL_ROCM_OFFLOAD_ARCH")
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "ROCm OpenMP offloading requires a Clang compiler, but got '${CMAKE_CXX_COMPILER_ID}' at '${CMAKE_CXX_COMPILER}'. "
            "Configure CMake with ROCm clang, e.g. -DCMAKE_CXX_COMPILER=/opt/rocm/llvm/bin/clang++"
        )
    endif()
    message(STATUS "ROCm OpenMP offloading enabled for arch: ${OPENBIMRL_ROCM_OFFLOAD_ARCH}")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fopenmp-targets=amdgcn-amd-amdhsa --offload-arch=${OPENBIMRL_ROCM_OFFLOAD_ARCH}"
    )
endif()

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

option(OPENBIMRL_USE_PREBUILT_IFCOPENSHELL
       "Use a preinstalled IfcOpenShell prefix instead of FetchContent" OFF)
set(OPENBIMRL_IFCOPENSHELL_PREFIX "/opt/ifcopenshell"
    CACHE PATH "IfcOpenShell install prefix (include/, lib/)")

option(OPENBIMRL_STATIC_IFCOPENSHELL
       "Build/link IfcOpenShell as static libs into OpenBIMRL_Native (FetchContent path)" OFF)

option(OPENBIMRL_BUILD_NATIVE_TESTS "Build C++ gtest suite" OFF)

# Prefer shared by default; static IfcOpenShell bundling is opt-in (#2).
if(OPENBIMRL_STATIC_IFCOPENSHELL AND NOT OPENBIMRL_USE_PREBUILT_IFCOPENSHELL)
    set(BUILD_SHARED_LIBS OFF)
else()
    set(BUILD_SHARED_LIBS ON)
endif()

set(SCHEMA_VERSIONS "2x3" "4" "4x3_add2")

find_library(GMP_LIBRARY_FILE NAMES gmp mpir REQUIRED)
cmake_path(GET GMP_LIBRARY_FILE PARENT_PATH GMP_LIBRARY_DIR)
set(MPFR_LIBRARY_DIR ${GMP_LIBRARY_DIR})

FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

if(NOT OpenCASCADE_DIR)
    foreach(_occ_cmake_dir
            "/usr/lib/x86_64-linux-gnu/cmake/opencascade"
            "/usr/lib/cmake/opencascade")
        if(EXISTS "${_occ_cmake_dir}/OpenCASCADEConfig.cmake")
            set(OpenCASCADE_DIR "${_occ_cmake_dir}" CACHE PATH "OpenCASCADE CMake package directory")
            break()
        endif()
    endforeach()
endif()

find_package(OpenCASCADE REQUIRED)
find_package(Eigen3 REQUIRED)

include(OcctCompat)

if(OPENBIMRL_USE_PREBUILT_IFCOPENSHELL)
    message(STATUS "Using prebuilt IfcOpenShell from ${OPENBIMRL_IFCOPENSHELL_PREFIX}")

    set(_ifcos_lib_dirs
        "${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib"
        "${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib/x86_64-linux-gnu")

    if(OPENBIMRL_STATIC_IFCOPENSHELL)
        find_library(_openbimrl_ifc_parse_lib NAMES libIfcParse.a IfcParse
                     PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)
        find_library(_openbimrl_ifc_geom_lib NAMES libIfcGeom.a IfcGeom
                     PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)
        set(_openbimrl_ifc_import_type STATIC)
    else()
        find_library(_openbimrl_ifc_parse_lib NAMES IfcParse
                     PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)
        find_library(_openbimrl_ifc_geom_lib NAMES IfcGeom
                     PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)
        set(_openbimrl_ifc_import_type SHARED)
    endif()
    find_library(_openbimrl_ifc_kernel_lib NAMES geometry_kernel_opencascade
                 PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)

    add_library(IfcParse ${_openbimrl_ifc_import_type} IMPORTED GLOBAL)
    set_target_properties(IfcParse PROPERTIES
        IMPORTED_LOCATION "${_openbimrl_ifc_parse_lib}")

    add_library(IfcGeom ${_openbimrl_ifc_import_type} IMPORTED GLOBAL)
    set_target_properties(IfcGeom PROPERTIES
        IMPORTED_LOCATION "${_openbimrl_ifc_geom_lib}")

    add_library(geometry_kernel_opencascade STATIC IMPORTED GLOBAL)
    set_target_properties(geometry_kernel_opencascade PROPERTIES
        IMPORTED_LOCATION "${_openbimrl_ifc_kernel_lib}")

    set(ifcopenshell_SOURCE_DIR "${OPENBIMRL_IFCOPENSHELL_PREFIX}/include")
else()
    if(OPENBIMRL_STATIC_IFCOPENSHELL)
        message(STATUS "Building IfcOpenShell as static libraries (OPENBIMRL_STATIC_IFCOPENSHELL=ON)")
    endif()
    set(BUILD_GEOMSERVER OFF)
    set(BUILD_IFCPYTHON OFF)
    set(BUILD_CONVERT OFF)
    set(BUILD_EXAMPLES OFF)
    set(BUILD_DOCUMENTATION OFF)
    set(COLLADA_SUPPORT OFF)
    set(GLTF_SUPPORT OFF)
    set(IFCXML_SUPPORT OFF)
    set(USD_SUPPORT OFF)
    set(HDF5_SUPPORT OFF)
    set(BUILD_IFCGEOM ON)
    set(WITH_CGAL OFF)

    FetchContent_Declare(
            ifcopenshell
            GIT_REPOSITORY https://github.com/IfcOpenShell/IfcOpenShell.git
            GIT_TAG eafa158ca0cd5ba2ca22b5e588b0375cab2efbce
            SOURCE_SUBDIR cmake
    )

    FetchContent_MakeAvailable(ifcopenshell)

    if(_openbimrl_lib_compat_from)
        _openbimrl_patch_occt_link_paths()
        foreach(_extra_target IfcGeom geometry_kernel_opencascade)
            _openbimrl_patch_target_link_paths("${_extra_target}")
        endforeach()
    endif()
endif()

set(OPENBIMRL_CORE_SOURCES
        src/model/session.cpp
        src/geometry/service.cpp
        src/geometry/mesh_and_polygon.cpp
        src/geometry/element_frame.cpp
        src/properties/snapshot.cpp
        src/properties/ifc_data_utils.cpp
        src/properties/ifc_data_getters.cpp
        src/properties/ifc_data_handlers.cpp
        src/pathfinding/edge_costs.cpp
)

set(OPENBIMRL_FFI_SOURCES
        src/ffi/rule_context.cpp
        src/ffi/session_and_init.cpp
        src/ffi/ops.cpp
)

add_library(openbimrl_core STATIC ${OPENBIMRL_CORE_SOURCES})
set_property(TARGET openbimrl_core PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(openbimrl_core PUBLIC IFOPSH_WITH_OPENCASCADE)

target_include_directories(openbimrl_core PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src/properties"
    "/usr/include/opencascade"
    "/usr/include/oce")

if(OPENBIMRL_USE_PREBUILT_IFCOPENSHELL)
    target_include_directories(openbimrl_core PUBLIC
        "${OPENBIMRL_IFCOPENSHELL_PREFIX}/include")
else()
    FetchContent_GetProperties(ifcopenshell)
    target_include_directories(openbimrl_core PUBLIC
        "${ifcopenshell_SOURCE_DIR}/src")
endif()

target_link_libraries(openbimrl_core PUBLIC
        TKernel TKMath TKBRep TKGeomBase TKGeomAlgo TKG3d TKG2d
        TKShHealing TKTopAlgo TKMesh TKPrim TKBool TKBO TKFillet
        TKXSBase TKOffset TKHLR
        nlohmann_json::nlohmann_json
        IfcParse IfcGeom geometry_kernel_opencascade
        Eigen3::Eigen
        OpenMP::OpenMP_CXX)

add_library(OpenBIMRL_Native SHARED ${OPENBIMRL_FFI_SOURCES})
set_property(TARGET OpenBIMRL_Native PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(OpenBIMRL_Native PUBLIC openbimrl_core)
if(OPENBIMRL_STATIC_IFCOPENSHELL)
    # Keep IfcOpenShell symbols inside the JNI/JNA shared object when possible.
    set_target_properties(OpenBIMRL_Native PROPERTIES
        LINK_FLAGS "-Wl,--exclude-libs,ALL")
endif()

if(OPENBIMRL_BUILD_NATIVE_TESTS)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    enable_testing()

    add_executable(OpenBIMRL_Native_UnitTest
            test/unit/edge_costs_test.cpp)
    target_link_libraries(OpenBIMRL_Native_UnitTest PRIVATE
            openbimrl_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(OpenBIMRL_Native_UnitTest)

    add_definitions(-DRESOURCES_DIR="${CMAKE_SOURCE_DIR}/test/integration/resources")
    add_executable(OpenBIMRL_Native_Test
            test/integration/test.cpp
            test/integration/ifc_elements.h)
    target_link_libraries(OpenBIMRL_Native_Test PRIVATE
            OpenBIMRL_Native GTest::gtest_main)
    gtest_discover_tests(OpenBIMRL_Native_Test)
endif()
