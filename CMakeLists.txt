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

option(OPENBIMRL_ENABLE_CUDA_OFFLOAD
       "Enable NVIDIA CUDA OpenMP GPU offloading (experimental / WIP)" OFF)
set(OPENBIMRL_CUDA_OFFLOAD_ARCH "" CACHE STRING
    "CUDA offload architecture, e.g. sm_89 (passed to --offload-arch)")

# Env / Bazel action_env can supply the arch when the cache value is empty
# (matches historical Makefile + Dockerfile.dev OPENBIMRL_ROCM_OFFLOAD_ARCH).
if(NOT OPENBIMRL_ROCM_OFFLOAD_ARCH AND DEFINED ENV{OPENBIMRL_ROCM_OFFLOAD_ARCH}
        AND NOT "$ENV{OPENBIMRL_ROCM_OFFLOAD_ARCH}" STREQUAL "")
    set(OPENBIMRL_ROCM_OFFLOAD_ARCH "$ENV{OPENBIMRL_ROCM_OFFLOAD_ARCH}")
endif()
if(NOT OPENBIMRL_CUDA_OFFLOAD_ARCH AND DEFINED ENV{OPENBIMRL_CUDA_OFFLOAD_ARCH}
        AND NOT "$ENV{OPENBIMRL_CUDA_OFFLOAD_ARCH}" STREQUAL "")
    set(OPENBIMRL_CUDA_OFFLOAD_ARCH "$ENV{OPENBIMRL_CUDA_OFFLOAD_ARCH}")
endif()

if(OPENBIMRL_ENABLE_ROCM_OFFLOAD AND OPENBIMRL_ENABLE_CUDA_OFFLOAD)
    message(FATAL_ERROR
        "Enable only one of OPENBIMRL_ENABLE_ROCM_OFFLOAD or OPENBIMRL_ENABLE_CUDA_OFFLOAD")
endif()

if(OPENBIMRL_ENABLE_ROCM_OFFLOAD)
    if(NOT OPENBIMRL_ROCM_OFFLOAD_ARCH)
        set(_openbimrl_rocminfo "/opt/rocm/bin/rocminfo")
        if(EXISTS "${_openbimrl_rocminfo}")
            execute_process(
                COMMAND ${_openbimrl_rocminfo}
                OUTPUT_VARIABLE _openbimrl_rocminfo_out
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            string(REGEX MATCH "gfx[0-9a-z]+" OPENBIMRL_ROCM_OFFLOAD_ARCH
                   "${_openbimrl_rocminfo_out}")
        endif()
        if(NOT OPENBIMRL_ROCM_OFFLOAD_ARCH)
            message(FATAL_ERROR
                "OPENBIMRL_ENABLE_ROCM_OFFLOAD=ON requires OPENBIMRL_ROCM_OFFLOAD_ARCH "
                "(e.g. gfx1100), or a working ${_openbimrl_rocminfo} with a visible GPU")
        endif()
        message(STATUS "Detected ROCm offload arch: ${OPENBIMRL_ROCM_OFFLOAD_ARCH}")
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

# NVIDIA path mirrors ROCm structurally. Device offload with host clang + CUDA
# toolkit is experimental; prefer nvcc.dockerfile / --config=cuda_offload for CI
# scaffolding. See https://developer.nvidia.com/cuda-llvm-compiler
if(OPENBIMRL_ENABLE_CUDA_OFFLOAD)
    # Optional arch: empty → try nvidia-smi (mirrors ROCm/rocminfo).
    if(NOT OPENBIMRL_CUDA_OFFLOAD_ARCH)
        find_program(_openbimrl_nvidia_smi nvidia-smi)
        if(_openbimrl_nvidia_smi)
            execute_process(
                COMMAND ${_openbimrl_nvidia_smi}
                    --query-gpu=compute_cap --format=csv,noheader
                OUTPUT_VARIABLE _openbimrl_cuda_caps
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
            # "8.9" → sm_89; take first GPU if several.
            string(REGEX REPLACE "\n.*" "" _openbimrl_cuda_cap "${_openbimrl_cuda_caps}")
            string(REGEX REPLACE "\\." "" _openbimrl_cuda_sm "${_openbimrl_cuda_cap}")
            if(_openbimrl_cuda_sm MATCHES "^[0-9]+$")
                set(OPENBIMRL_CUDA_OFFLOAD_ARCH "sm_${_openbimrl_cuda_sm}")
            endif()
        endif()
        if(NOT OPENBIMRL_CUDA_OFFLOAD_ARCH)
            message(FATAL_ERROR
                "OPENBIMRL_ENABLE_CUDA_OFFLOAD=ON requires OPENBIMRL_CUDA_OFFLOAD_ARCH "
                "(e.g. sm_89), or a working nvidia-smi with a visible GPU")
        endif()
        message(STATUS "Detected CUDA offload arch: ${OPENBIMRL_CUDA_OFFLOAD_ARCH}")
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR
            "CUDA OpenMP offloading requires a Clang compiler, but got '${CMAKE_CXX_COMPILER_ID}' at '${CMAKE_CXX_COMPILER}'. "
            "Configure with system or CUDA-capable clang, e.g. -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
        )
    endif()
    message(WARNING
        "CUDA OpenMP offload is experimental/WIP; flags may require a CUDA-capable "
        "clang and further cmake work. Arch=${OPENBIMRL_CUDA_OFFLOAD_ARCH}")
    message(STATUS "CUDA OpenMP offloading enabled for arch: ${OPENBIMRL_CUDA_OFFLOAD_ARCH}")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fopenmp-targets=nvptx64-nvidia-cuda --offload-arch=${OPENBIMRL_CUDA_OFFLOAD_ARCH}"
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

# Optional local source (Bazel-vendored) skips the URL download.
# Accepts either the json source root or a path to its CMakeLists.txt.
set(OPENBIMRL_JSON_SOURCE_DIR "" CACHE PATH
    "Local nlohmann/json source dir (or CMakeLists.txt); empty uses FetchContent URL")
if(OPENBIMRL_JSON_SOURCE_DIR)
    if(EXISTS "${OPENBIMRL_JSON_SOURCE_DIR}/CMakeLists.txt")
        set(_openbimrl_json_root "${OPENBIMRL_JSON_SOURCE_DIR}")
    elseif(EXISTS "${OPENBIMRL_JSON_SOURCE_DIR}" AND
           "${OPENBIMRL_JSON_SOURCE_DIR}" MATCHES "CMakeLists\\.txt$")
        get_filename_component(_openbimrl_json_root "${OPENBIMRL_JSON_SOURCE_DIR}" DIRECTORY)
    else()
        message(FATAL_ERROR
            "OPENBIMRL_JSON_SOURCE_DIR must be a nlohmann/json source tree or its CMakeLists.txt, got: ${OPENBIMRL_JSON_SOURCE_DIR}")
    endif()
    FetchContent_Declare(json SOURCE_DIR "${_openbimrl_json_root}")
else()
    FetchContent_Declare(
            json
            URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
endif()
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
    # Keep IfcOpenShell discoverable after the .so is copied out of the build tree (JNA).
    set(CMAKE_INSTALL_RPATH "${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

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

# rules_foreign_cc (and plain cmake --install) package the JNA shared library + C ABI header.
install(TARGETS OpenBIMRL_Native
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)
install(FILES include/openbimrl_c_api.h DESTINATION include)

if(OPENBIMRL_BUILD_NATIVE_TESTS)
    # BUILD_SHARED_LIBS is ON for IfcOpenShell / OpenBIMRL_Native. FetchContent
    # inherits that, so googletest would build libgtest*.so and the test binary
    # would fail at runtime unless those dirs are on LD_LIBRARY_PATH. Keep gtest
    # static so it is linked into the executables.
    set(_openbimrl_saved_build_shared_libs "${BUILD_SHARED_LIBS}")
    set(BUILD_SHARED_LIBS OFF)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
    set(BUILD_SHARED_LIBS "${_openbimrl_saved_build_shared_libs}")
    unset(_openbimrl_saved_build_shared_libs)
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
    # gtest_discover_tests (POST_BUILD) execs the binary at build time; the
    # loader must find libOpenBIMRL_Native.so in the build dir (same place as
    # the test). PRE_TEST defers discovery to ctest; BUILD_RPATH covers both.
    set_target_properties(OpenBIMRL_Native_Test PROPERTIES
            BUILD_RPATH "${CMAKE_BINARY_DIR}"
            BUILD_RPATH_USE_ORIGIN TRUE
            INSTALL_RPATH "$ORIGIN")
    gtest_discover_tests(OpenBIMRL_Native_Test
            DISCOVERY_MODE PRE_TEST
            PROPERTIES
                ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}")
endif()
