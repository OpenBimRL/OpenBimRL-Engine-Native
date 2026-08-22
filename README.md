# OpenBimRL Engine Native

C++ shared library (`OpenBIMRL_Native`) for the OpenBimRL engine: IFC loading and
geometry via **IfcOpenShell** / **Open CASCADE**, OpenMP pathfinding, and related
helpers. Entry points are exported through a stable **C ABI**
(`include/openbimrl_c_api.h`) so the JVM can load the `.so` with **JNA** (no C++
name mangling).

Bazel module name: **`openbimrl_native`**. Platform today: **linux/amd64** only.

## Responsibilities

- IFC model loading and geometry iteration (world coordinates)
- Element filtering, bounding boxes, building bounds, 2D polygons
- OpenMP pathfinding over extracted segments (optional ROCm / CUDA offload)
- Element frame extraction (`getElementFrame`) from placement / extrusion
- JSON serialization helpers for IFC entities

## Requirements

| Need | Notes |
|------|--------|
| OS / arch | Linux **amd64** (`x86_64`) |
| Compiler | System **LLVM clang** + **libomp** (`/usr/bin/clang`, `/usr/bin/clang++`) |
| Build | [Bazelisk](https://github.com/bazelbuild/bazelisk) (`bazel`) |
| IfcOpenShell | Prebuilt prefix (DevContainer / CI: `/opt/ifcopenshell`) |
| OCCT, Eigen, GMP, MPFR | System packages (e.g. distro OCCT + Eigen/GMP/MPFR) |

Optional:

- **ROCm** LLVM at `/opt/rocm/llvm` — `--config=rocm_offload`
- **CUDA** toolkit — `--config=cuda_offload` (WIP)

### DevContainer / local images

Same layout as the Engine repo — pick one Dockerfile and point
`.devcontainer/devcontainer.json` at it (`build.dockerfile`):

| File | Base | Use |
|------|------|-----|
| [`llvm.dockerfile`](llvm.dockerfile) | Ubuntu 24.04 + system clang | Default CPU (default in `devcontainer.json`) |
| [`rocm.dockerfile`](rocm.dockerfile) | ROCm + system clang | `--config=rocm_offload` |
| [`nvcc.dockerfile`](nvcc.dockerfile) | `nvidia/cuda:13.3.1-cudnn-devel-ubuntu24.04` | `--config=cuda_offload` (WIP) |

```bash
docker build -f llvm.dockerfile -t openbimrl-native:llvm .
docker build -f rocm.dockerfile -t openbimrl-native:rocm .
docker build -f nvcc.dockerfile -t openbimrl-native:nvcc .
```

Prebuilt IfcOpenShell is expected at `/opt/ifcopenshell` (not baked into these
images). For GPU variants, add `"runArgs": ["--gpus", "all"]` in
`devcontainer.json`.

## Layout

```
.
├── BUILD.bazel / MODULE.bazel / .bazelrc
├── CMakeLists.txt          # Used by Bazel (rules_foreign_cc) and standalone CMake
├── cmake/                  # OCCT compat helpers
├── include/
│   ├── openbimrl_c_api.h   # Public C ABI for JNA
│   └── openbimrl/          # C++ domain headers
├── src/
│   ├── model/              # IfcSession, GUID, queries
│   ├── geometry/           # iterators, bounds, polygon, frames
│   ├── properties/         # IFC property snapshot + JSON
│   ├── pathfinding/        # Pure edge-cost algorithms
│   └── ffi/                # C ABI adapters + RuleContext
├── test/
│   ├── unit/               # No IFC fixtures (Bazel + CMake)
│   └── integration/        # IFC fixtures (CMake/ctest only)
└── .github/workflows/      # bazel-test.yml, release.yml
```

## Build (Bazel)

From **this directory** (module root):

```bash
# Default: host clang + libomp → libOpenBIMRL_Native.so
bazel build //:openbimrl_native

# ROCm OpenMP GPU offload
OPENBIMRL_ROCM_OFFLOAD_ARCH=gfx1100 \
  bazel build //:openbimrl_native --config=rocm_offload

# CUDA OpenMP offload (WIP)
OPENBIMRL_CUDA_OFFLOAD_ARCH=sm_89 \
  bazel build //:openbimrl_native --config=cuda_offload
```

Expects `OPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON` and
`OPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell` (set in `BUILD.bazel`).

Configs and compiler pins live in [`.bazelrc`](.bazelrc); the `cmake()` target
and offload `select`s are in [`BUILD.bazel`](BUILD.bazel).

## Tests

### Bazel unit tests (CI)

No IfcOpenShell / OCCT. Pathfinding gtests only:

```bash
bazel test //:edge_costs_test
# or: bazel test //:native_unit_tests
# or: bazel test //...
```

[`.github/workflows/bazel-test.yml`](.github/workflows/bazel-test.yml) runs
`bazel test //:native_unit_tests` on `ubuntu-24.04` (push/PR to `main`).

### CMake / ctest (local; unit + IFC integration)

```bash
cmake -B build -S . \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DOPENBIMRL_BUILD_NATIVE_TESTS=ON \
  -DOPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON \
  -DOPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell
cmake --build build -j --target OpenBIMRL_Native_UnitTest OpenBIMRL_Native_Test
ctest --test-dir build --output-on-failure
```

IFC integration tests need fixtures under `test/integration/resources/`. On CI
they run in [`release.yml`](.github/workflows/release.yml) as `test-integration`
(after `build-llvm`, reusing the shared IfcOpenShell prefix).

## Releases

[`.github/workflows/release.yml`](.github/workflows/release.yml) publishes
versioned `.so` assets to a GitHub Release on push to `main` (and
`workflow_dispatch`). The `test-integration` job (CMake IFC gtests) must pass
before the release job runs.

**Version scheme:** UTC calendar date **`YYYY.MM.DD`** (dots). Git tag: `vYYYY.MM.DD`.

| Asset | Build |
|-------|--------|
| `libOpenBimRL-Engine-Native-x86_64-llvm-<version>.so` | Default Bazel (clang + libomp) |
| `libOpenBimRL-Engine-Native-x86_64-rocm-<version>.so` | `--config=rocm_offload` |
| `libOpenBimRL-Engine-Native-x86_64-nvcc-<version>.so` | CUDA devel image (`cuda_offload` WIP) |

Pipeline: one shared IfcOpenShell build (`build-ifcopenshell` →
`ifcopenshell-prefix` artifact), then llvm / rocm / nvcc jobs Bazel-build the
native `.so` only. All on the ubuntu 24.04 family for glibc/libstdc++
compatibility.

**CI limitations (no GPU on free runners):**

- **rocm:** compile-only with `OPENBIMRL_ROCM_OFFLOAD_ARCH=gfx1100`
- **nvcc:** CUDA-environment build; device offload codegen is not verified yet

Dated release assets are published on this repository’s GitHub Releases. Downstream
JVM consumers typically rename/copy a chosen variant to a stable unversioned
classpath resource before loading it with JNA.

## Consumers (JNA)

The public surface is the C ABI in `include/openbimrl_c_api.h`. Typical CMake
output is `libOpenBIMRL_Native.so`; release assets use the dated
`libOpenBimRL-Engine-Native-x86_64-<variant>-<version>.so` names above. Load via
JNA (or equivalent) against that ABI — this repo does not ship the JVM side.

## Standalone CMake (optional)

Useful for iteration or IFC integration tests without driving Bazel. Bazel’s
`cmake()` target still uses the same `CMakeLists.txt`.

```bash
cmake -B build -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DOPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON \
  -DOPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell
cmake --build build -j --target OpenBIMRL_Native
```

When IfcOpenShell is not on the default library path:

```bash
export LD_LIBRARY_PATH="${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `OPENBIMRL_USE_PREBUILT_IFCOPENSHELL` | `OFF` | Use a preinstalled IfcOpenShell prefix |
| `OPENBIMRL_IFCOPENSHELL_PREFIX` | `/opt/ifcopenshell` | Prefix with `include/` and `lib/` |
| `OPENBIMRL_STATIC_IFCOPENSHELL` | `OFF` | Link IfcParse/IfcGeom statically into `OpenBIMRL_Native` |
| `OPENBIMRL_JSON_SOURCE_DIR` | _(empty)_ | Local nlohmann/json tree; Bazel sets via `@nlohmann_json_src` |
| `OPENBIMRL_BUILD_NATIVE_TESTS` | `OFF` | Build gtest executables |
| `OPENBIMRL_ENABLE_ROCM_OFFLOAD` | `OFF` | ROCm OpenMP GPU offloading |
| `OPENBIMRL_ROCM_OFFLOAD_ARCH` | _(empty)_ | e.g. `gfx1100` (else `rocminfo`) |
| `OPENBIMRL_ENABLE_CUDA_OFFLOAD` | `OFF` | NVIDIA CUDA OpenMP offloading (WIP) |
| `OPENBIMRL_CUDA_OFFLOAD_ARCH` | _(empty)_ | e.g. `sm_89` (else `nvidia-smi`) |

## Exported API (C ABI)

Domain logic lives under `src/{model,geometry,properties,pathfinding}`; `src/ffi`
marshals to the C ABI in `openbimrl_c_api.h`.

Current entry points:

- `initIfc`, `init_function`
- `filterByElement`, `filterByGUID`
- `getBoundingBox`, `getElementFrame`, `calculatingBuildingBounds`
- `request_geometry_polygon`, `copy_geometry_polygon`
- `request_ifc_object_json_size`, `ifc_object_to_json`
- `calculate_path_edge_costs`

To add a function: implement under `src/`, add a thin adapter in `src/ffi/`,
declare it in `openbimrl_c_api.h`, and list the source in `CMakeLists.txt`
(Bazel picks up sources via the `cmake_srcs` glob).

## Pointers

| Path | Purpose |
|------|---------|
| [`BUILD.bazel`](BUILD.bazel) | `openbimrl_native` cmake target, unit tests, offload configs |
| [`MODULE.bazel`](MODULE.bazel) | Bzlmod deps (`rules_foreign_cc`, googletest, nlohmann/json) |
| [`.bazelrc`](.bazelrc) | `rocm_offload` / `cuda_offload` compiler env |
| [`CMakeLists.txt`](CMakeLists.txt) | Library + optional gtests / offload flags |
| [`.github/workflows/bazel-test.yml`](.github/workflows/bazel-test.yml) | Fast pathfinding unit-test CI |
| [`.github/workflows/release.yml`](.github/workflows/release.yml) | llvm / rocm / nvcc `.so` releases + IFC integration tests |
