# OpenBimRL Engine Native

C++ shared library (`OpenBIMRL_Native`) wrapping **IfcOpenShell** and **Open CASCADE** for IFC geometry. Callable entry points are exported through the **C ABI** (`extern "C"` in `include/functions.h`): stable symbol names and a plain calling convention so foreign runtimes (e.g. JNA on the JVM) can `dlopen` the library and invoke functions without C++ name mangling.

## Responsibilities

- IFC model loading and geometry iteration (world coordinates)
- Element filtering, bounding boxes, building bounds, 2D polygons
- OpenMP pathfinding over extracted segments
- Element frame extraction (`getElementFrame`) from placement / extrusion
- JSON serialization helpers for IFC entities

## Layout

```
.
├── CMakeLists.txt          # CMake project
├── include/                # Public headers (functions, types, utils)
├── functions/              # Exported C entry points
├── utils/                  # IfcOpenShell helpers, pathfinding, element frames
└── test/                   # Optional gtest suite + IFC fixtures
```

## Dependencies

| Dependency | Purpose |
|------------|---------|
| [IfcOpenShell](https://github.com/IfcOpenShell/IfcOpenShell) | IFC parsing and geometry kernel |
| Open CASCADE (OCCT) | B-rep / mesh operations |
| Eigen3 | Linear algebra (pathfinding) |
| GMP / MPFR | IfcOpenShell numeric support |
| OpenMP | Parallel pathfinding |

## Build

Configure and build from this directory:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j --target OpenBIMRL_Native
```

The shared library is written to `build/libOpenBIMRL_Native.so` (exact path depends on the generator and platform).

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `OPENBIMRL_USE_PREBUILT_IFCOPENSHELL` | `OFF` | Use a preinstalled IfcOpenShell prefix instead of FetchContent |
| `OPENBIMRL_IFCOPENSHELL_PREFIX` | `/opt/ifcopenshell` | Prefix with `include/` and `lib/` when prebuilt mode is on |
| `OPENBIMRL_BUILD_NATIVE_TESTS` | `OFF` | Build the gtest executable `OpenBIMRL_Native_Test` |
| `OPENBIMRL_ENABLE_ROCM_OFFLOAD` | `OFF` | Enable ROCm OpenMP GPU offloading (requires ROCm Clang) |
| `OPENBIMRL_ROCM_OFFLOAD_ARCH` | _(empty)_ | GPU arch when offloading is on, e.g. `gfx1100` |

Example with a preinstalled IfcOpenShell:

```bash
cmake -B build -S . \
  -DOPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON \
  -DOPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell

cmake --build build -j --target OpenBIMRL_Native
```

When IfcOpenShell is not on the default system path, set `LD_LIBRARY_PATH` before running tests or loading the library:

```bash
export LD_LIBRARY_PATH="${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
```

## Tests

```bash
cmake -B build -S . \
  -DOPENBIMRL_BUILD_NATIVE_TESTS=ON \
  -DOPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON \
  -DOPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell

cmake --build build -j --target OpenBIMRL_Native_Test
ctest --test-dir build --output-on-failure
```

Fixtures live under `test/resources/` (see `test/resources/correct.ifc`; other fixtures may be gitignored).

## Exported API (C ABI)

Graph functions are implemented in C++ but exposed to callers through the **C application binary interface**. Declarations live in `include/functions.h` inside `extern "C" { … }`; implementations are under `functions/`. That keeps exported symbols unmangled (e.g. `getBoundingBox`) and limits the cross-language surface to C-compatible types.

Inputs and outputs use the shared callback protocol in `functions/function.cpp` (`Functions::init_function`), which is also defined in terms of C function pointers and plain data buffers.

Current entry points:

- `filterByElement`, `filterByGUID`
- `getBoundingBox`, `getElementFrame`
- `calculatingBuildingBounds`

To add a function: create `functions/<name>.cpp`, list it in `CMakeLists.txt`, and add an `extern "C"` declaration in `include/functions.h`.
