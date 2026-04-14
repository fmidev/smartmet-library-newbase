# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`newbase` is a C++ library (`smartmet-library-newbase`) that defines and implements **QueryData** (`.sqd`), FMI's native binary format for gridded meteorological data. It is a foundational dependency of SmartMet Server and most FMI weather tools.

## Build commands

```bash
make                  # Build libsmartmet-newbase.so and .a (also builds Python bindings)
make test             # Run all C++ tests + Python binding tests
make format           # clang-format all source and test files
make clean            # Clean build artifacts
make rpm              # Build RPM package
```

### Running a single test

```bash
# Build and run one test (from repo root)
make -C test NFmiFastQueryInfoTest && ./test/NFmiFastQueryInfoTest

# Or from the test/ directory
cd test && make NFmiGridTest && ./NFmiGridTest
```

Tests link against the locally-built `../libsmartmet-newbase.so`, not the system-installed version. Test data lives in `test/data/` (`.sqd` files, masks, config files).

### Sanitizer builds

```bash
make ASAN=yes         # AddressSanitizer (disables Python bindings)
make TSAN=yes         # ThreadSanitizer
```

### Dependencies

Build-requires: `smartmet-library-macgyver`, `smartmet-library-gis`, GDAL, Boost (iostreams, regex, thread), fmt 12.x. GDAL can be disabled with `DISABLED_GDAL=yes`.

## Architecture

### The 4D data model

QueryData is a 4-dimensional float array: **parameters × locations × levels × times**. Memory layout has time varying fastest (most cache-friendly to iterate times in the inner loop).

Missing values use the sentinel `kFloatMissing`.

### Key classes

| Class | Role |
|-------|------|
| `NFmiQueryData` | On-disk container. Owns raw data (`NFmiRawData`) and descriptors. Handles file I/O. |
| `NFmiQueryInfo` | Base iterator for navigating the 4 dimensions via reset/next pattern. |
| `NFmiFastQueryInfo` | Preferred subclass — adds index-based access, bulk reads (`GetValues()`), cached interpolation. |

**Always prefer `NFmiFastQueryInfo`** over `NFmiQueryInfo` for new code.

### Descriptor classes (all inherit `NFmiDataDescriptor`)

| Descriptor | Dimension | Wraps |
|-----------|-----------|-------|
| `NFmiParamDescriptor` | Parameters (temperature, wind, ...) | `NFmiParamBag` of `NFmiDataIdent` |
| `NFmiHPlaceDescriptor` | Horizontal locations (grid or stations) | `NFmiArea` + `NFmiGrid` or `NFmiLocationBag` |
| `NFmiVPlaceDescriptor` | Vertical levels (pressure, height, hybrid) | `NFmiLevelBag` |
| `NFmiTimeDescriptor` | Valid times, origin time, forecast period | `NFmiTimeBag` |

### Iteration pattern

All dimensions use reset/next semantics:
```cpp
qi->ResetParam();
while (qi->NextParam()) {
  qi->ResetLocation();
  while (qi->NextLocation()) { ... }
}
```

### Projection/area system

`NFmiArea` is the base class for coordinate projections. Subclasses include `NFmiLatLonArea`, `NFmiStereographicArea`, `NFmiLambertConformalConicArea`, `NFmiMercatorArea`, `NFmiWebMercatorArea`, `NFmiGdalArea`, and others. `NFmiAreaFactory` creates areas from string descriptions.

### Thread safety

Multiple `NFmiFastQueryInfo` iterators can safely share the same read-only `NFmiQueryData`. Individual iterators are **not** thread-safe — each thread needs its own.

### Python bindings

The `python/` directory contains pybindgen-based bindings exposing ~40 classes as the `newbase` Python module. Generated from `newbase-binding.py`.

```bash
make -C python test   # Run Python binding tests
```

## Code conventions

- All classes use the `NFmi` prefix (legacy FMI naming convention).
- Source files live in `newbase/`, headers and implementations together.
- Test framework is FMI's `regression/tframe.h` (not Boost.Test). Each test file is a standalone executable.
- File format: `.clang-format` — Google-based style, Allman braces, 100-column limit.
