# OpenBimRL Engine Native

C++ shared library (`OpenBIMRL_Native`) wrapping **IfcOpenShell** and **Open CASCADE** for IFC geometry. Callable entry points are exported through the **C ABI** (`include/openbimrl_c_api.h`): stable symbol names and a plain calling convention so foreign runtimes (e.g. JNA on the JVM) can `dlopen` the library and invoke functions without C++ name mangling.

## Responsibilities

- IFC model loading and geometry iteration (world coordinates)
- Element filtering, bounding boxes, building bounds, 2D polygons
- OpenMP pathfinding over extracted segments
- Element frame extraction (`getElementFrame`) from placement / extrusion
- JSON serialization helpers for IFC entities

## Layout

```
.
├── CMakeLists.txt
├── cmake/                  # OCCT compat helpers
├── include/
│   ├── openbimrl_c_api.h   # Public C ABI for JNA
│   └── openbimrl/          # C++ domain headers
│       ├── model/
│       ├── geometry/
│       ├── properties/
│       ├── pathfinding/
│       └── ffi/
├── src/
│   ├── model/              # IfcSession, GUID, queries
│   ├── geometry/           # iterators, bounds, polygon, frames
│   ├── properties/         # IFC property snapshot + JSON
│   ├── pathfinding/        # Pure edge-cost algorithms
│   └── ffi/                # C ABI adapters + RuleContext
└── test/
    ├── unit/               # No IFC fixtures
    └── integration/        # IFC fixtures + end-to-end
```

## Dependencies

| Dependency | Purpose |
|------------|---------|
| [IfcOpenShell](https://github.com/IfcOpenShell/IfcOpenShell) | IFC parsing and geometry kernel |
| Open CASCADE (OCCT) | B-rep / mesh operations |
| Eigen3 | Linear algebra |
| GMP / MPFR | IfcOpenShell numeric support |
| OpenMP | Parallel pathfinding |

## Build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j --target OpenBIMRL_Native
```

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `OPENBIMRL_USE_PREBUILT_IFCOPENSHELL` | `OFF` | Use a preinstalled IfcOpenShell prefix |
| `OPENBIMRL_IFCOPENSHELL_PREFIX` | `/opt/ifcopenshell` | Prefix with `include/` and `lib/` |
| `OPENBIMRL_STATIC_IFCOPENSHELL` | `OFF` | Link IfcParse/IfcGeom statically into `OpenBIMRL_Native` (FetchContent builds static libs; prebuilt needs `.a`) |
| `OPENBIMRL_BUILD_NATIVE_TESTS` | `OFF` | Build gtest executables |
| `OPENBIMRL_ENABLE_ROCM_OFFLOAD` | `OFF` | ROCm OpenMP GPU offloading |
| `OPENBIMRL_ROCM_OFFLOAD_ARCH` | _(empty)_ | GPU arch, e.g. `gfx1100` |

Example with preinstalled IfcOpenShell:

```bash
cmake -B build -S . \
  -DOPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON \
  -DOPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell
cmake --build build -j --target OpenBIMRL_Native
```

When IfcOpenShell is not on the default system path:

```bash
export LD_LIBRARY_PATH="${OPENBIMRL_IFCOPENSHELL_PREFIX}/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
```

## Tests

```bash
cmake -B build -S . \
  -DOPENBIMRL_BUILD_NATIVE_TESTS=ON \
  -DOPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON \
  -DOPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell
cmake --build build -j --target OpenBIMRL_Native_UnitTest OpenBIMRL_Native_Test
ctest --test-dir build --output-on-failure
```

## Exported API (C ABI)

Graph functions are implemented in C++ but exposed through the **C application binary interface** in `openbimrl_c_api.h`. Domain logic lives under `src/{model,geometry,properties,pathfinding}`; `src/ffi` only marshals.

Current entry points:

- `initIfc`, `init_function`
- `filterByElement`, `filterByGUID`
- `getBoundingBox`, `getElementFrame`, `calculatingBuildingBounds`
- `request_geometry_polygon`, `copy_geometry_polygon`
- `request_ifc_object_json_size`, `ifc_object_to_json`
- `calculate_path_edge_costs`

To add a function: implement domain logic under `src/`, add a thin adapter in `src/ffi/`, declare it in `openbimrl_c_api.h`, and list the source in `CMakeLists.txt`.
