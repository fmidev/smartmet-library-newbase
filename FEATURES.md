# smartmet-library-newbase — Feature List

A structured inventory of capabilities provided by the newbase
library. Use as a checklist when drafting release notes. When new
functionality is added, append the new entry under the matching
section (and bump the *Last updated* line at the bottom).

`smartmet-library-newbase` is the **QueryData** library — FMI's native
binary format for gridded meteorological data (`.sqd`). It is one of
the oldest, largest, and most foundational dependencies in the
SmartMet ecosystem (~196 public headers, all under the legacy `NFmi`
prefix). The library produces both shared
(`libsmartmet-newbase.so`) and static (`libsmartmet-newbase.a`)
artefacts, plus a pybindgen-based Python module.

---

## 1. QueryData file format

- **Native FMI gridded data format** — `.sqd` files.
- **4-D data model** — `parameters × locations × levels × times`.
- **Memory layout** — time varies fastest (cache-friendly for time
  iteration).
- **Missing values** — `kFloatMissing` sentinel.
- **`NFmiQueryData`** — top-level on-disk container with raw data
  (`NFmiRawData`) plus descriptors and file I/O.
- **Binary I/O** — read/write `.sqd` files directly.
- **Documentation** — `docs/querydata.md` user guide.

## 2. Data navigation

- **`NFmiQueryInfo`** — base iterator over the 4-D structure (reset
  / next).
- **`NFmiFastQueryInfo`** — preferred subclass with:
  - Index-based access.
  - Bulk reads (`GetValues()`).
  - Cached interpolation handles.
- **Reset/next idiom**:
  ```cpp
  qi->ResetParam();
  while (qi->NextParam()) { qi->ResetLocation();
    while (qi->NextLocation()) { ... } }
  ```
- **`NFmiQueryInfoSatel`** — variant for satellite data.

## 3. Descriptors

All inherit `NFmiDataDescriptor`. Each owns one of the four data
dimensions:

- **`NFmiParamDescriptor`** — parameters; wraps `NFmiParamBag` of
  `NFmiDataIdent`.
- **`NFmiHPlaceDescriptor`** — horizontal locations (grid or
  stations); wraps `NFmiArea + NFmiGrid` or `NFmiLocationBag`.
- **`NFmiVPlaceDescriptor`** — vertical levels; wraps `NFmiLevelBag`.
- **`NFmiTimeDescriptor`** — valid times, origin time, forecast
  period; wraps `NFmiTimeBag`.

## 4. Projection / area system

`NFmiArea` is the abstract base for coordinate projections. Built-in
subclasses:

- **`NFmiLatLonArea`** — geographic.
- **`NFmiRotatedLatLonArea`** — rotated pole.
- **`NFmiMercatorArea`** / **`NFmiWebMercatorArea`** — Mercator
  family.
- **`NFmiStereographicArea`** / **`NFmiAzimuthalArea`** /
  **`NFmiEquidistArea`** / **`NFmiOrthographicArea`** /
  **`NFmiGnomonicArea`** / **`NFmiLambertEqualArea`** — azimuthal
  family.
- **`NFmiLambertConformalConicArea`** — Lambert Conformal Conic.
- **`NFmiKKJArea`** / **`NFmiYKJArea`** / **`NFmiPKJArea`** —
  Finnish historical projections.
- **`NFmiGdalArea`** — GDAL-backed wrapper for arbitrary projections.
- **`NFmiAreaFactory`** — build an `NFmiArea` from a string
  description, EPSG code, proj4 string or WKT.
- **`NFmiAreaTools`** — area-manipulation helpers.

## 5. Grid representation

- **`NFmiGrid`** — regular grid built on top of an `NFmiArea`.
- **`NFmiGridBase`** — grid abstraction.
- **`NFmiBox`** — bounding box.

## 6. Parameters

- **`NFmiParam`** — parameter (id, name, missing-value, limits).
- **`NFmiDataIdent`** — parameter + producer pair.
- **`NFmiParamBag`** — collection of `NFmiDataIdent`.
- **`NFmiParameterName`** — well-known parameter ID enumeration.
- **`NFmiCombinedParam`** — derived parameter from multiple raw
  parameters.
- **`NFmiEditParam`** — editable parameter overlay.

## 7. Producers & levels

- **`NFmiProducer`** — data producer (id + name).
- **`NFmiProducerName`** — well-known producer IDs.
- **`NFmiProducerIdLister`** — producer enumeration helper.
- **`NFmiLevel`**, **`NFmiLevelBag`**, **`NFmiLevelType`** — vertical
  level types (pressure, height, hybrid, ...).

## 8. Locations & stations

- **`NFmiLocation`**, **`NFmiLocationBag`** — point locations for
  station data.
- **`NFmiLocationFinder`** — lookup by name / coordinates.
- **`NFmiPoint`**, **`NFmiVector`** — 2-D coordinate primitives.

## 9. Time

- **`NFmiTime`** / **`NFmiMetTime`** — base time / met-time
  (rounded to model timestep).
- **`NFmiTimeBag`** — time-range with regular steps.
- **`NFmiTimeList`** — irregular time list.
- **`NFmiTimePeriod`** — time interval helpers.
- **`NFmiTimeDescriptor`** — origin time + valid times for a
  QueryData.
- **`NFmiTimeZoneFinder`** — timezone lookup by coordinates.

## 10. Interpolation

- **`NFmiInterpolation`** — point, bilinear, biquadratic, nearest,
  inverse-distance, and other interpolators.
- **Cached interpolation** — `NFmiFastQueryInfo` reuses interpolation
  coefficients for repeated queries to the same location.

## 11. Area masks

A reusable masking system used for area-restricted calculations:

- **`NFmiAreaMask`** — abstract mask interface.
- **`NFmiAreaMaskImpl`** — base implementation.
- **`NFmiAreaMaskList`** — composite masks.
- **`NFmiBitmapAreaMask`** / **`NFmiBitMask`** — bitmap-backed mask.
- **`NFmiCalculatedAreaMask`** — formula-driven mask.
- **`NFmiAreaMaskHelperStructures`** — supporting helpers.
- **`NFmiMaskedDataIterator`** — iterate only over mask-true cells.

## 12. Calculations & modifiers

- **`NFmiCalculator`** — value-modification engine over a
  `NFmiQueryInfo`.
- **`NFmiCalculationCondition`** — conditional masking for
  calculations.
- **`NFmiParamDataModifier`** / **`NFmiParamDataModifierList`** —
  apply value transforms (unit conversion, scaling, offsets).
- **`NFmiDataModifierCombinedParam`** — combined-parameter modifier.

## 13. Mathematical & meteorological helpers

- **`NFmiMetMath`** — meteorological math (e.g. wind components,
  potential temperature, mixing ratios).
- **`NFmiAngle`** — angle arithmetic with wraparound.
- **`NFmiVector`**, **`NFmiPoint`** — vector / point math.

## 14. String & I/O utilities

- **`NFmiString`** / **`NFmiStringList`** / **`NFmiStringTools`** —
  legacy string helpers.
- **`NFmiCommentStripper`** — strip `#` comments from config files.
- **`NFmiCmdLine`** — command-line argument parser.
- **`NFmiSettings`** / **`NFmiSettingsImpl`** — `.conf`-style settings
  reader with `value("key", default)` lookup.

## 15. Sanitiser & sanitised builds

- **AddressSanitizer**: `make ASAN=yes` (disables Python bindings).
- **ThreadSanitizer**: `make TSAN=yes`.
- **GDAL-less build**: `make DISABLED_GDAL=yes` for stripped-down
  builds that skip `NFmiGdalArea`.

## 16. Thread safety

- **Read-only `NFmiQueryData` is safe to share** across threads.
- **Each thread must own its own `NFmiFastQueryInfo`** iterator —
  iterators are not thread-safe.

## 17. Python bindings

- **`newbase` Python module** built under `python/`.
- **pybindgen-generated** from `newbase-binding.py`.
- **~40 classes exposed** — `NFmiQueryData`, `NFmiFastQueryInfo`,
  `NFmiArea`, `NFmiPoint`, `NFmiTime`, `NFmiLatLonArea`, etc.
- **Build alongside the C++ library**: `make` builds both.
- **Test**: `make -C python test` runs `test_newbase.py` against the
  bundled `test_data.sqd.xz`.

## 18. Examples

Under `examples/`:

- **`example_NFmiPoint`** / **`example_NFmiVector`** — basic math
  primitives.
- **`example_NFmiSettings`** / **`example_NFmiSettingsImpl`** —
  settings file usage.
- **`example_NFmiMetMath`** — meteorological math.
- **`example_NFmiStringList`** — string list.
- **`example_NFmiTimeZoneFinder`** — timezone lookup (with
  `timezones.txt`).
- **`example_Landscaping`** — landscaping use case.
- **`bugtest_NFmiMetTime`** — regression test for time arithmetic.
- **Sample configs** — `ukko.conf`, `odin.conf`, `windows.conf`.

## 19. Testing

- **~32 standalone test executables** under `test/*Test.cpp`.
- **Framework**: FMI's own `regression/tframe.h` (not Boost.Test).
- **Per-test build**:
  `make -C test NFmiFastQueryInfoTest && ./test/NFmiFastQueryInfoTest`.
- **Test data** — `.sqd` files, masks, and config files under
  `test/data/`.
- **Python tests**: `make -C python test`.

## 20. Build & integration

- **Library outputs**: `libsmartmet-newbase.so` and
  `libsmartmet-newbase.a`.
- **CMake support**: a `CMakeLists.txt` + `cmake/` directory alongside
  the hand-written `Makefile`.
- **Build**: `make` (release) / `make` with `ASAN=yes` / `TSAN=yes` /
  `DISABLED_GDAL=yes`.
- **Format**: `make format` runs clang-format.
- **Install**: `make install`.
- **RPM**: `make rpm`.
- **Cloud Build**: a `cloudbuild.yaml` for Google Cloud Build is
  shipped.
- **CI**: CircleCI on RHEL 8 / RHEL 10 (`fmidev/smartmet-cibase-{8,10}`
  Docker images).
- **Dependencies**: `smartmet-library-macgyver`,
  `smartmet-library-gis`, GDAL (optional), Boost (iostreams, regex,
  thread), fmt 12.x.
- **Public headers** installed under `/usr/include/smartmet/newbase/`.

---

*Last updated: 2026-06-01.*
