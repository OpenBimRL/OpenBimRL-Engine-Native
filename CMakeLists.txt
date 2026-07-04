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

# Find OpenMP
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

set(BUILD_SHARED_LIBS ON)

option(OPENBIMRL_USE_PREBUILT_IFCOPENSHELL
       "Use a preinstalled IfcOpenShell prefix instead of FetchContent" OFF)
set(OPENBIMRL_IFCOPENSHELL_PREFIX "/opt/ifcopenshell"
    CACHE PATH "IfcOpenShell install prefix (include/, lib/)")

option(OPENBIMRL_BUILD_NATIVE_TESTS "Build C++ gtest suite" OFF)

set(SCHEMA_VERSIONS "2x3" "4" "4x3_add2")

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

find_package(OpenCASCADE REQUIRED)
find_package(Eigen3 REQUIRED)

# Debian OCCT packages reference unversioned .so names that are not shipped.
set(_openbimrl_lib_compat "${CMAKE_BINARY_DIR}/lib-compat")
set(_openbimrl_lib_compat_from "")
set(_openbimrl_lib_compat_to "")
macro(_openbimrl_add_lib_compat_symlink _link_name _real_path)
    if(EXISTS "${_real_path}" AND NOT EXISTS "/usr/lib/x86_64-linux-gnu/${_link_name}")
        file(MAKE_DIRECTORY "${_openbimrl_lib_compat}")
        set(_compat_path "${_openbimrl_lib_compat}/${_link_name}")
        if(NOT EXISTS "${_compat_path}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${_real_path}" "${_compat_path}")
        endif()
        list(APPEND _openbimrl_lib_compat_from
             "/usr/lib/x86_64-linux-gnu/${_link_name}")
        list(APPEND _openbimrl_lib_compat_to "${_compat_path}")
    endif()
endmacro()

_openbimrl_add_lib_compat_symlink("libtbb.so" "/usr/lib/x86_64-linux-gnu/libtbb.so.12")
_openbimrl_add_lib_compat_symlink("libtbbmalloc.so"
                                  "/usr/lib/x86_64-linux-gnu/libtbbmalloc.so.2")
_openbimrl_add_lib_compat_symlink("libtcl.so" "/usr/lib/x86_64-linux-gnu/libtcl8.6.so")
_openbimrl_add_lib_compat_symlink("libtk.so" "/usr/lib/x86_64-linux-gnu/libtk8.6.so")

if(_openbimrl_lib_compat_from)
    link_directories("${_openbimrl_lib_compat}")

    macro(_openbimrl_apply_lib_compat_replacements _links_str _new_links_var)
        set(_result "${_links_str}")
        list(LENGTH _openbimrl_lib_compat_from _compat_count)
        math(EXPR _last_compat_index "${_compat_count} - 1")
        foreach(_idx RANGE ${_last_compat_index})
            list(GET _openbimrl_lib_compat_from ${_idx} _from)
            list(GET _openbimrl_lib_compat_to ${_idx} _to)
            string(REPLACE "${_from}" "${_to}" _result "${_result}")
        endforeach()
        set(${_new_links_var} "${_result}")
    endmacro()

    macro(_openbimrl_patch_target_link_paths _target)
        if(NOT TARGET "${_target}")
            return()
        endif()
        foreach(_prop INTERFACE_LINK_LIBRARIES LINK_LIBRARIES)
            get_target_property(_links "${_target}" ${_prop})
            if(_links AND NOT _links STREQUAL "${_prop}-NOTFOUND")
                list(JOIN _links ";" _links_str)
                _openbimrl_apply_lib_compat_replacements("${_links_str}" _new_links)
                if(NOT _new_links STREQUAL _links_str)
                    set_target_properties("${_target}" PROPERTIES ${_prop}
                                          "${_new_links}")
                endif()
            endif()
        endforeach()
    endmacro()

    macro(_openbimrl_patch_occt_link_paths)
        file(GLOB _occt_cmake_files
             "/usr/lib/x86_64-linux-gnu/cmake/opencascade/OpenCASCADE*Targets*.cmake")
        foreach(_occt_file IN LISTS _occt_cmake_files)
            file(READ "${_occt_file}" _occt_contents)
            string(REGEX MATCHALL "add_library\\([^ )]+" _occt_targets "${_occt_contents}")
            foreach(_occt_entry IN LISTS _occt_targets)
                string(REGEX REPLACE "add_library\\(" "" _occt_target "${_occt_entry}")
                _openbimrl_patch_target_link_paths("${_occt_target}")
            endforeach()
        endforeach()
    endmacro()

    _openbimrl_patch_occt_link_paths()
endif()

if(OPENBIMRL_USE_PREBUILT_IFCOPENSHELL)
    message(STATUS "Using prebuilt IfcOpenShell from ${OPENBIMRL_IFCOPENSHELL_PREFIX}")

    set(_ifcos_lib_dirs
        "${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib"
        "${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib/x86_64-linux-gnu")

    find_library(_openbimrl_ifc_parse_lib NAMES IfcParse
                 PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)
    find_library(_openbimrl_ifc_geom_lib NAMES IfcGeom
                 PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)
    find_library(_openbimrl_ifc_kernel_lib NAMES geometry_kernel_opencascade
                 PATHS ${_ifcos_lib_dirs} REQUIRED NO_DEFAULT_PATH)

    add_library(IfcParse SHARED IMPORTED GLOBAL)
    set_target_properties(IfcParse PROPERTIES
        IMPORTED_LOCATION "${_openbimrl_ifc_parse_lib}")

    add_library(IfcGeom SHARED IMPORTED GLOBAL)
    set_target_properties(IfcGeom PROPERTIES
        IMPORTED_LOCATION "${_openbimrl_ifc_geom_lib}")

    add_library(geometry_kernel_opencascade STATIC IMPORTED GLOBAL)
    set_target_properties(geometry_kernel_opencascade PROPERTIES
        IMPORTED_LOCATION "${_openbimrl_ifc_kernel_lib}")

    set(ifcopenshell_SOURCE_DIR "${OPENBIMRL_IFCOPENSHELL_PREFIX}/include")
else()
    # configure IfcOpenShell (trim unused targets)
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
        utils/pathfinding_omp.cpp
        utils/guid.cpp
        utils/ifc_geom_utils.cpp
)

set_property(TARGET OpenBIMRL_Native PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(OpenBIMRL_Native PRIVATE IFOPSH_WITH_OPENCASCADE)

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
        PUBLIC geometry_kernel_opencascade
)

if(OPENBIMRL_USE_PREBUILT_IFCOPENSHELL)
    target_include_directories(OpenBIMRL_Native PUBLIC
        "${OPENBIMRL_IFCOPENSHELL_PREFIX}/include")
else()
    FetchContent_GetProperties(ifcopenshell)
    target_include_directories(OpenBIMRL_Native PUBLIC
        "${ifcopenshell_SOURCE_DIR}/src")
endif()

target_include_directories(OpenBIMRL_Native PUBLIC
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/utils"
    "/usr/include/opencascade"
    "/usr/include/oce")

target_link_libraries(OpenBIMRL_Native PUBLIC Eigen3::Eigen)

if(OPENBIMRL_BUILD_NATIVE_TESTS)
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
endif()
