# Native EPSG:3067 (ETRS-TM35FIN) area — impact analysis

**Status:** design / pre-implementation. **Decision taken:** add a native transverse-mercator
`NFmiArea` subclass so EPSG:3067 stops falling back to `NFmiGdalArea`.

## 1. Why

- FMI radar is served entirely in **EPSG:3067** (all 234 `/smartmet/radar/geotiff/*` products; ODIM
  composites likewise). It is not an edge case — it is 100 % of the radar hot path.
- Today `NFmiArea::DetectClassId` (`NFmiArea.cpp:47-113`) only recognises the legacy **YKJ / EPSG:2393**
  `tmerc` signature (`ellps=intl`, `x_0=3500000`, specific `towgs84`) → `kNFmiYKJArea`. ETRS-TM35FIN
  (`ellps=GRS80`, `x_0=500000`, `k=0.9996`, no `towgs84`) matches nothing and hits the
  `return kNFmiGdalArea;` fallback at `NFmiArea.cpp:112`.
- `NFmiGdalArea` is ABI-frozen, "known-wobbly", routes every transform through PROJ
  (`Fmi::CoordinateTransformation`), and constructs from proj.db/SQLite — the source of the PROJ/SQLite
  deadlock already fixed once in gis. A native closed-form class removes PROJ from the transform and
  the proj.db hit from construction, and removes the fragile class from the flagship path.
- 3067 is Finland's national CRS, so the class benefits all Finnish gridded data, not only radar.

## 2. Serialization / compatibility — resolved

Querydata persists the area by a **numeric class-id tag**, not a projection string. `NFmiGrid`
(`NFmiGrid.cpp:452` write / `:491` read) and `NFmiHPlaceDescriptor` (`:1168` / `:1240`) write
`itsArea->ClassId()` and rebuild via `CreateSaveBase(classId)`, which **throws on an unknown id**
(`NFmiSaveBaseFactory.cpp` default case). `NFmiGdalArea`'s datum/description strings are only its *body*;
the class is still selected by the id (8).

Consequences, and why this is acceptable:

- **New build reading old files:** always fine (existing ids unchanged).
- **Old build reading a NEW-id file:** fails (`unable to create unknown class`). This is the only risk.
- **In-memory, on-the-fly conversion (the radar engine reading GeoTIFF/ODIM into an in-process
  `NFmiQueryData`) never persists** — no version boundary is crossed. **Safe from day one.**
- **Persisted `.sqd`:** the only writer that matters is our own. Note `h5toqd` *is* a persist-to-disk
  path and builds its area through `CreateFromCorners → CreateFromBBox → Create(SR) → DetectClassId`,
  so flipping `DetectClassId` also flips `h5toqd`'s 3067 output to the new id. That is fine provided the
  **newbase upgrade is deployed fleet-wide (all readers) before any persisting converter emits the new
  id.** Since this is a newbase change, the whole stack rebuilds against it anyway — normal release
  ordering, not a blocker.

**Rule (rollout order):** upgrade the **consumers first — SmartMet Server (and every other newbase
reader) — then the data-production side** (`h5toqd`/`geotiff2qd` and any tool that persists 3067 `.sqd`)
flips to emit id 85. Never the reverse. The radar engine (in-memory, S2) does not wait for any of this.

## 3. What to build

A **general** `NFmiTransverseMercatorArea`, parameterized by (ellipsoid a/f, `lon0`, `k0`,
false easting/northing) — not a 3067-specific hack. EPSG:3067 = {GRS80, lon0=27, k0=0.9996, FE=500000,
FN=0, lat0=0}. YKJ can later become a thin subclass/config of it.

Math reuse: ~80–90 % liftable from `NFmiKKJArea` (base of `NFmiYKJArea`), which already has
`SetupAnyEllipsoid(a, f)` (GRS80 is a one-call swap) and the Krüger forward/inverse series. Genuine
additions:
1. **k0 scaling** — KKJ assumes k0=1 (`ynorth = a1*ksi`); TM35FIN needs 0.9996 on the meridian/easting
   terms (and inverse).
2. **De-zone the geometry** — replace KKJ's 6°-zone / `x_0 = 1000000*(6+zone) - 5500000` arithmetic
   (`NFmiKKJArea.cpp:417,443,570,571`) with a plain (lon0, FE, FN).
3. **Recompute the hard-coded `1.0040685` inverse constant** (`NFmiKKJArea.cpp:431`) — it is
   International-1924-specific and would be wrong for GRS80; derive it from the ellipsoid.
4. **Set EPSG:3067 identity** — `itsProjStr = "EPSG:3067"`, `itsSpatialReference =
   make_shared<Fmi::SpatialReference>(3067)`, and emit correct ETRS89/GRS80/TM35FIN `WKT()`. Do **not**
   reuse the KKJ `towgs84` strings.

## 4. Migration checklist (highest-risk first)

All in newbase unless noted. Verified items are marked ✓.

1. ✓ **`NFmiSaveBaseFactory.h`** — add `kNFmiTransverseMercatorArea = 85` (slot 85 is free; ceiling
   `KNFmiMaxClass=99`; avoid deprecated slot 12). **`NFmiSaveBaseFactory.cpp`** — add the `case`
   returning `new NFmiTransverseMercatorArea`. *Without this, `.sqd` written with the class cannot be
   read back.*
2. **`NFmiArea.cpp:47-113` `DetectClassId`** — add a 3067/GRS80 `tmerc` branch. It must sit **inside the
   `if(ok)` datum block** (ETRS89 is WGS84-aligned so `ok` is true), *not* in the YKJ `else if` chain.
   Add the matching `case` in **`Create()` (`:128-196`)**, or it still falls to `default: new
   NFmiGdalArea`.
3. ✓ **`NFmiArea.cpp:915-958` `HashValueKludge`** — add the new class to the `dynamic_cast` chain (place
   it outside the `#ifndef DISABLED_GDAL` guard; the class needs no GDAL). `HashValue` is non-virtual
   (ABI freeze), so a missing entry silently falls to the base `HashValue()` → **hash collisions →
   querydata-engine coordinate-cache corruption.** Give the class its own `HashValue()`.
4. **Correct virtual accessors** — `SpatialReference()`, `ProjStr()`, `WKT()`, `LatLonToWorldXY` /
   `WorldXYToLatLon` (+ `ToXY`/`ToLatLon`/`XYToWorldXY`/`WorldXYToXY`/`WorldRect`), `NewArea`, `Clone`,
   `operator==`, `AreaStr`, `Write`/`Read` (call `NFmiArea::Write/Read` first). Correct
   `LatLonToWorldXY`/`WorldXYToLatLon` transparently satisfy `NFmiArea::TrueNorthAzimuth` (generic base
   impl; `NFmiGdalArea` does not override it) and thus wind-component rotation.
5. **`NFmiAreaFactory.cpp:857-990` `tmerc` branch** — currently hard-codes YKJ constants and throws on
   deviation. Relax to read lon0/lat0/k/x_0/ellps as parameters and dispatch YKJ vs generic TM by the
   (ellps, k, x_0) triple, so string specs can build the native class.
6. **`brainstorm/plugins/q3/q3/SQD_Projection.h:41`** — string-matches `ClassName() == "kNFmiGdalArea"`
   to decide the `"FMI|"` projection prefix. A native class returns a different ClassName, changing this
   branch for 3067. **Review**: add the new class name or verify the non-prefixed path round-trips.
7. **Optional (no regression if skipped)** — download plugin output writers: `GeoTiffStreamer`
   (relies on `WKT()` being correct — works once #4 is right), `GribStreamer`/`NetCdfStreamer` (3067
   already throws `default:` today; add a `case` only if TM output is wanted; NetCDF's YKJ special-case
   is *not* reused — route through the generic `setSpheroidAndWKT(area->WKT())`). `Query.h:127`
   aliases `A_TransverseMercator = kNFmiYKJArea` (latent confusion, request-side only). `qdinfo.cpp:481`
   prints `ClassName()` — cosmetic, may shift a regression fixture.

**Clean elsewhere:** qengine, imagine, imagine2, gis, smarttools have no relevant class-id branching or
subclass down-casts. `NFmiQueryDataUtil` azimuthal-orientation checks and the many
`== kNFmiLatLonArea` "is-latlon" tests do not affect a (non-latlon, non-azimuthal) TM area.

## 5. qengine — no code changes

The querydata engine is area-agnostic: no `NFmiGdalArea` reference, no `dynamic_cast`, no `ClassId`
branching. It obtains the CRS only via `QImpl::SpatialReference()` → `NFmiFastQueryInfo` →
`NFmiArea::SpatialReference()`, reprojects with `Fmi::CoordinateTransformation`, and computes grid-north
via `Fmi::OGR::gridNorth` off the SpatialReference (the old `TrueNorthAzimuth` calls are commented out,
`Q.cpp:201,258`). Correctness is entirely contingent on the native area returning:

- a valid `SpatialReference()` with a **stable, WKT-based `hashValue()`** (feeds the coordinate-cache
  key at `EngineImpl.cpp:856-877` and producer grouping via `gridHashValue()`),
- correct `WKT()`/`ProjStr()` (metadata/content reports; GeoTIFF/NetCDF output WKT),
- correct `LatLon()`/`CoordinateMatrix()`/world-XY, `GridHashValue()`, and `CreateFromBBox` support.

Spots to test (not change): `getWorldCoordinatesForSR` SR-match shortcut, `project_coordinates`,
`WindUMS/WindVMS/GridNorth` for a `relative_uv=true` 3067 producer, `QImpl::sample()` to a 3067 target.

## 6. Validation plan

- Unit test the class against PROJ using the exact TM35FIN corners observed in production files, e.g.
  `radar_finland_dbz` UL `(-196593.004, 8084432.005)` and the WGS84 extent GDAL reports; round-trip
  `LatLonToWorldXY`→`WorldXYToLatLon` to sub-mm.
- Regression vs the current `NFmiGdalArea` 3067 output for the same file (same lon/lat ↔ XY, same
  `WKT()`/`hashValue()` where it must match cache keys).
- `.sqd` write → read round-trip with the native id; confirm an old-build reader throws (documents the
  fleet-ordering rule) and a new build reconstructs identically.

## 7. Fit with the radar-off-GeoServer effort

This class is one pillar of migrating radar rendering from GeoServer to SmartMet Server. The others,
already scoped from source recon:

- **Reader (assembly, not new code):** GeoTIFF radar decode already exists in
  `qdless/QdlessGdalRasterSource` (reads the ODIM-style `gain/offset/nodata/undetect` from
  `GDAL_METADATA`, valid time from the `YYYYMMDDHHMM` filename); ODIM HDF5 decode in
  `qdtools/Hdf5File` + `qdtools/main/h5toqd.cpp` (`opera_name_to_newbase`, projdef→area, gain/offset,
  COMP/IMAGE + PVOL). Factor the shared logic into one reusable "radar → in-memory NFmiQueryData"
  library; it powers both the batch tools and the engine.
- **Serving path:** monochrome radar is a 1-parameter `NFmiQueryData`, so it rides qengine's Q/contour/
  raster/GetFeatureInfo path unchanged. Two options that share the same reader:
  - **S1** — batch-convert to `.sqd` + plain qengine (today's accrate/fmippn pattern). Ships fastest,
    zero server code; persists (see §2 rule), and ×4 disk (byte→float).
  - **S2** — read GeoTIFF/ODIM directly in a qengine `type=` extension; no disk doubling, no lag; the
    "beat GeoServer at scale" path. **S2 is where the native area pays off with zero file-format risk.**
- **Value model:** scaled byte, dBZ = raw·0.5 − 32, nodata=255→missing, undetect=0. Identical quartet
  in GeoTIFF metadata and ODIM attributes → one decoder. Not paletted → genuinely contourable.
- **Memory fork:** the `radar_finland_dbz` composite is 4963×7316 ≈ 145 MB/frame as float32; radar
  animation wants large `number_to_keep`. Measure early; if prohibitive for big composites, evolve those
  producers to a byte/overview-backed Model (GeoTIFFs are tiled 256² DEFLATE with 5 overviews).
- **Layer migration:** GeoServer layer specs come from opengeoweb presets + GetCapabilities (geometry/
  time/CRS/style names) + GeoServer REST (SLD colour ramps, ImageMosaic dir + time regex). The mosaic
  stores should point at these same `geotiff/` dirs, making the switch a re-point.

## 8. Open decisions

- **S1 vs S2 first.** Recommendation: S1 for high-traffic composites to migrate layers now, while
  building the reader lib + qengine `type=` (S2) behind it. S1 is a genuine stepping stone (shared
  reader), not throwaway.
- **q3 `SQD_Projection.h:41`** ClassName handling — confirm the intended round-trip behaviour.
- **Storage representation** for big composites (float qd vs byte/overview) — decide after measuring.
