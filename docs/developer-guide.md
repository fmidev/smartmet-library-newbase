# newbase developer guide

This guide is for developers who change `smartmet-library-newbase`, or use it in other
code. newbase implements QueryData, FMI's native format for gridded and point data, together
with the projections, times, parameters and tools that come with it. The querydata engine,
the timeseries, WMS and download plugins, textgen and the qdtools programs are built on it.

[querydata.md](querydata.md) is the usage guide (reading, iterating, interpolating,
cross-sections), and [CLAUDE.md](../CLAUDE.md) has the build summary. This guide covers
what happens underneath: the file format, how data is stored in memory, the area
(projection) classes, times, parameters, and the behaviour to watch out for.

## Contents

1. [Building and testing](#1-building-and-testing)
2. [The file format](#2-the-file-format)
3. [Opening and storing data](#3-opening-and-storing-data)
4. [Infos: navigating the data](#4-infos-navigating-the-data)
5. [Parameters](#5-parameters)
6. [Areas and projections](#6-areas-and-projections)
7. [Times](#7-times)
8. [Writing data](#8-writing-data)
9. [Other components](#9-other-components)
10. [Compatibility](#10-compatibility)
11. [Known pitfalls](#11-known-pitfalls)

---

## 1. Building and testing

```bash
make                                              # library + Python bindings
make test                                         # C++ tests and the Python binding tests
make -C test NFmiFastQueryInfoTest && ./test/NFmiFastQueryInfoTest
make ASAN=yes / make TSAN=yes                     # sanitizer builds
```

The tests use `regression/tframe.h` and the data files in `test/data/`: `hiladata.sqd`
(grid, also gzip- and bzip2-compressed), `pistedata.fqd` (stations), radar data, masks and
settings files. `DISABLED_GDAL=yes` builds without GDAL, which removes `NFmiGdalArea`.

## 2. The file format

A QueryData file (`.sqd` for grids, `.fqd` for points, optionally `.gz` / `.bz2`) is a
**text header** followed by the **data pool**.

The header is written by `NFmiQueryInfo::Write()`:

1. the magic string `@$°£Q`, the four bytes of the integer `0x4f464e49` (it reads `INFO` in
   files written on little-endian machines, `OFNI` on big-endian ones), `@$°£`;
2. `VER 7`: the info version (`DefaultFmiInfoVersion` in `NFmiVersion.h`);
3. the class id and name, and four reserved zeros;
4. the header text and post-processing string lists;
5. the four descriptors, in this order: parameters, horizontal places (area and grid, or
   the station list), vertical places (levels), times.

The data pool is `float32` values, written in binary (`itsSaveAsBinaryFlag`, the
default since version 6) or as text in very old files. The values are ordered with **time
varying fastest**, then level, location and parameter:

```
index = param * (locations*levels*times) + location * (levels*times) + level * times + time
```

The byte order of the writing machine is recorded, and data from a machine of the other
endianness is byte-swapped on reading.

## 3. Opening and storing data

`NFmiQueryData(path, memoryMap = true)` opens a file:

* A **directory** is resolved by `NFmiFileSystem::FindQueryData()`: the newest
  (by modification time) non-empty `*.sqd` / `*.fqd` file, compressed or not, ignoring
  dot files. `-` means standard input.
* Uncompressed files of version 6 or later in the native byte order are **memory
  mapped** read-only (`NFmiRawData`, through `Fmi::MappedFile`). The file size is checked
  against the header before mapping.
* Compressed files, old versions, byte-swapped data, and `memoryMap = false` are read
  into memory.

`NFmiRawData` holds the values, behind a `std::shared_mutex`: readers take the shared
lock, writers the exclusive one. The first write to memory-mapped data **copies the whole
data set into memory** (`Unmap()`), so the file itself is never modified.

The data set also caches, on first use, the latitude and longitude of every grid point
(`LatLonCache()`, built once with `boost::call_once`), and has `GridHashValue()` for
identifying the grid.

## 4. Infos: navigating the data

An info is an iterator: it holds a current parameter, location, level and time. Reading
uses the current position (`FloatValue()`) or interpolates around it.

* Use **`NFmiFastQueryInfo`**. It keeps the four indices directly and has the bulk
  accessors (`GetValues()`, `GetLevelToVec()`, `GetCube()`, the `Values()` and
  `GridValues()` families) and index-based interpolation (`GetLocationIndex()`,
  `GetTimeIndex()`, `CachedInterpolation()`, `FastPressureLevelValue()`).
* Creating an info **copies the descriptors**, including the whole station list for point
  data. It is cheap for grids and not cheap for large station data: reuse infos within a
  thread instead of creating one per value.
* **Threads.** One info per thread. Any number of infos can read the same `NFmiQueryData`
  at once.
* **Navigation calls return false** when the target is not in the data (`Param()`,
  `Time()`, `Level()`, `Location()`), and they leave that dimension at an invalid index.
  Check the return value.
* **Interpolation methods.** `InterpolatedValue(latlon)` interpolates bilinearly on grids,
  using the parameter's interpolation method, and handles missing corners. On point data it
  **moves the info to the nearest station, with no distance limit**, and returns its value.
  `InterpolatedValue(latlon, time)` also interpolates in time, `HeightValue()` and
  `PressureLevelValue()` vertically (pressure logarithmically).
* **Nearest station.** `NearestLocation()` and `Location(latlon)` on point data find the
  nearest station within `theMaxDistance`, whose default is effectively unlimited. Pass
  a limit (in metres) when a far-away station is not an acceptable answer.
* **Outside a grid.** `Location(latlon)` returns false and `InterpolatedValue()` returns
  `kFloatMissing` for points outside the grid area.

`kFloatMissing` (32700) marks missing values everywhere.

## 5. Parameters

Parameters are identified by `FmiParameterName` numbers (`NFmiParameterName.h`), for
example `kFmiTemperature` = 4. Each `NFmiParam` also carries a name, a value range,
scale and base, a precision, and an interpolation method.

**Combined parameters.** Two parameters pack several values into one float:
`kFmiTotalWind` (wind speed, direction, gust, u/v…, `NFmiTotalWind`) and
`kFmiWeatherAndCloudiness` (cloud amounts, precipitation form, type, intensity…,
`NFmiWeatherAndCloudiness`). When a parameter is not in the data directly,
`Param(id)` also looks for it among the sub-parameters of a combined parameter and, if
found, decodes it on each read. So `Param(kFmiWindSpeedMS)` can succeed on data that only
has `kFmiTotalWind`. Sub-parameter values interpolate differently (for example, weather
is a weighted combination of the neighbours, not a bilinear mean).

**Names.** `NFmiEnumConverter` maps parameter names to numbers and back
(`NFmiEnumConverterInit.cpp` holds the table). Its constructor builds the whole map, so
create one and share it (for example a function-local static); lookups do not modify it.
The parameter names stored inside data files are in Latin-1, not UTF-8.

## 6. Areas and projections

`NFmiArea` is the base of the projection classes. An area has three coordinate systems:

| Coordinates | Meaning |
|-------------|---------|
| lat/lon | Geographic, degrees. |
| WorldXY | Projected, metres (or degrees for lat/lon areas). |
| XY | A local rectangle (`XYArea()`), 0…1 by default; the grid maps it to grid indices. |

`ToXY()`, `ToLatLon()`, `LatLonToWorldXY()`, `WorldXYToLatLon()`, `XYToWorldXY()` and
`WorldXYToXY()` convert between them.

**Legacy and PROJ areas.** The legacy classes (`NFmiLatLonArea`, `NFmiStereographicArea`,
`NFmiMercatorArea`, `NFmiLambertConformalConicArea`, `NFmiRotatedLatLonArea`,
`NFmiYKJArea`, `NFmiTransverseMercatorArea`, …) keep their own serialisation format, so old
data files stay readable. Each of them also creates a `Fmi::SpatialReference` (from the gis
library) with a PROJ string; most of them are on the FMI sphere (`+R=6371220`,
`kRearth`). Anything else is an `NFmiGdalArea`, which works through PROJ entirely.

`NFmiArea::CreateFromBBox()`, `CreateFromCorners()`, `CreateFromCenter()` and friends create
an area from a spatial reference. They detect the legacy projections from the PROJ
parameters (`DetectClassId()` in `NFmiArea.cpp`) and create the legacy class when one
matches; EPSG:3067 (ETRS-TM35FIN) becomes the native `NFmiTransverseMercatorArea` (see
[epsg3067-native-area-impact.md](epsg3067-native-area-impact.md)).

**Area strings.** `NFmiAreaFactory::Create()` parses `projection:area[:grid]`:

```
stereographic,20,90,60:6,51.3,49,70.2          # FMI's Scandinavian editor area
latlon:10,20,30,40:10,10km                     # with the grid spacing
rotlatlon,-30:10,20,30,40
EPSG:3067|19,59,32,70                          # any PROJ-known CRS
FMI:<WKT or PROJ>|bl_lon,bl_lat,tr_lon,tr_lat  # NFmiGdalArea; corners on the FMI sphere
WGS84:<WKT or PROJ>|...                        # NFmiGdalArea; corners in WGS84
```

The area part is either the bottom-left and top-right corners, or a centre with a scale
(and an optional aspect ratio). `AreaStr()` produces a string that `Create()` reads back.
The comment at the top of `NFmiAreaFactory.cpp` lists every projection and its defaults.

Areas are compared with `operator==` and hashed with `HashValue()`. The querydata engine
uses `NFmiQueryData::GridHashValue()` (the hash of the horizontal descriptor) to
recognise data sets on the same grid.

## 7. Times

`NFmiMetTime` is a time with a **time step** (60 minutes by default), and the step affects
construction and arithmetic:

| Construction | Result |
|--------------|--------|
| `NFmiMetTime()` | now, rounded **down** to the step (the hour). |
| `NFmiMetTime(y, m, d, h, mi)` | rounded to the **nearest** step: 12:34 becomes 13:00. Pass `timeStep = 1` to keep minutes. |
| `NFmiMetTime(const Fmi::DateTime&)` | exact (one-minute step). |
| `NFmiMetTime::now()` | now, with minutes. |

`++` and `--` move by the step, and `SetTimeStep()` changes the step and, by default,
re-rounds the time. `NFmiMetTime` converts implicitly to and from `Fmi::DateTime` (UTC),
which is what new code should use at interfaces. Times are UTC.

The time descriptor holds either a regular `NFmiTimeBag` (first, last, step) or an
irregular `NFmiTimeList`, and the origin time (model run time).

## 8. Writing data

To create data: build the four descriptors, make an `NFmiQueryInfo` from them, and call
`NFmiQueryDataUtil::CreateEmptyData()` (or `NFmiQueryData(info)` + `Init()`). Then fill it
through an `NFmiFastQueryInfo` (`FloatValue(v)`, `SetValues()`, `SetLevelFromVec()`), and
`Write(filename)` it.

* `Write()` writes directly to the named file. Programs whose output is read by a running
  server must write to a temporary name in the same directory and rename it into place;
  otherwise readers can see a half-written file, and a mapped file that is rewritten in place
  crashes its readers with `SIGBUS`.
* `NFmiQueryData::Init(header, filename, initialize)` creates the output file as a
  memory-mapped, writable data pool, for data sets too large to build in memory.
* `Write()` writes the binary format unless `forceBinaryFormat = false` is passed.

## 9. Other components

| Component | Use |
|-----------|-----|
| `NFmiQueryDataUtil` | Combining, interpolating and filtering whole data sets; used heavily by qdtools. |
| `NFmiInfoAreaMask`, `NFmiCalculatedAreaMask`, `NFmiIndexMask*` | Masks for area calculations (smarttools, textgen). |
| `NFmiSettings` | A process-wide key/value configuration read from files through `NFmiPreProcessor` (`include <file>` lines, `#define`, `#` and `//` comments); used by textgen and the tools. The server uses libconfig instead. |
| `NFmiFileSystem` | File helpers, including `FindQueryData()`. |
| `NFmiLocationFinder`, `NFmiLocationBag`, `NFmiStation` | Named locations and station lists. |
| `NFmiMetMath`, `NFmiInterpolation` | Meteorological formulas and interpolation primitives. |
| `NFmiSvgPath`, `NFmiSvgTools` | SVG path geometry (for example area masks from paths). |
| `python/` | pybindgen bindings exposing about 40 classes as the `newbase` Python module. |

## 10. Compatibility

* **The file format is shared by every FMI tool** and by files archived years ago. Keep
  reading old versions working, and do not change what `Write()` produces without
  changing the info version and checking every reader.
* **Headers are public and many are inline** (`NFmiFastQueryInfo.h` has much of the
  navigation code inline). A layout or inline change affects every dependant: bump the spec
  version and the dependants' floors, and rebuild them together.
* Most classes have virtual methods; add new virtual methods at the end of a class.

## 11. Known pitfalls

* **Writing a data file in place breaks readers.** Write and rename (§8).
* **The first write to mapped data copies it all into memory** (§3).
* **The newest file in a directory is picked by modification time**, not by name or
  origin time.
* **Nearest-station lookups have no distance limit by default**, and
  `InterpolatedValue()` on point data returns a value however far the nearest station is
  (§4).
* **`InterpolatedValue()` on point data moves the info** to that station.
* **`NFmiMetTime(y, m, d, h, mi)` rounds to the nearest hour** unless you pass a time
  step (§7).
* **`Param(id)` may select a sub-parameter** of a combined parameter (§5).
* **Parameter names in files are Latin-1.**
* **`NFmiEnumConverter` is expensive to construct.** Share one.
* **WGS84 legacy projections become sphere projections.** A PROJ definition like
  `+proj=stere +datum=WGS84` is detected as a legacy projection and created as the
  corresponding FMI class, which works on a sphere; the positions differ slightly from a
  true ellipsoidal projection.
* **Creating an info copies station lists** (§4).
