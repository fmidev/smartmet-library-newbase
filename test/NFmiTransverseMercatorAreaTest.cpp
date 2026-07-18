// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiTransverseMercatorArea
 */
// ======================================================================

#include "NFmiTransverseMercatorArea.h"

#include <regression/tframe.h>

#include <gis/SpatialReference.h>

#include <cmath>
#include <memory>
#include <sstream>
#include <string>

// The default parameters describe ETRS-TM35FIN (EPSG:3067) on the WGS84 ellipsoid.
// Reference easting/northing values below are the authoritative EPSG:3067 corner
// coordinates read from production radar GeoTIFFs (gdalinfo), paired with the
// WGS84 lon/lat GDAL reports for those corners.

std::string tostr(const NFmiPoint& p)
{
  std::ostringstream out;
  out << std::fixed << "(" << p.X() << "," << p.Y() << ")";
  return out.str();
}

//! Protection against conflicts with global functions
namespace NFmiTransverseMercatorAreaTest
{
// A Finland-wide EPSG:3067 area (the corners only fix the local<->world scaling;
// the projection itself is independent of them).
NFmiTransverseMercatorArea finland()
{
  return NFmiTransverseMercatorArea(NFmiPoint(19, 59), NFmiPoint(32, 71));
}

// ----------------------------------------------------------------------

void create()
{
  NFmiTransverseMercatorArea area = finland();
  if (area.ClassId() != kNFmiTransverseMercatorArea)
    TEST_FAILED("Wrong class id");
  if (std::string(area.ClassName()) != "NFmiTransverseMercatorArea")
    TEST_FAILED("Wrong class name");
  TEST_PASSED();
}

// ----------------------------------------------------------------------
// On the central meridian (lon=27) the easting must equal the false easting
// exactly, and at the equator the northing must equal the false northing.
// This validates the central-meridian / false-easting / false-northing wiring.

void central_meridian()
{
  NFmiTransverseMercatorArea area = finland();

  NFmiPoint wxy = area.LatLonToWorldXY(NFmiPoint(27, 60));
  if (std::fabs(wxy.X() - 500000.0) > 1e-6)
    TEST_FAILED("Easting on central meridian should be 500000, got " + tostr(wxy));

  NFmiPoint origin = area.LatLonToWorldXY(NFmiPoint(27, 0));
  if (std::fabs(origin.X() - 500000.0) > 1e-6 || std::fabs(origin.Y() - 0.0) > 1e-6)
    TEST_FAILED("Projection origin (27,0) should be (500000,0), got " + tostr(origin));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
// Compare against authoritative EPSG:3067 values (from production GeoTIFFs).

void worldxy_reference()
{
  NFmiTransverseMercatorArea area = finland();

  struct Ref
  {
    double lon, lat, easting, northing;
  };
  // radar_finland_dbz (4 corners) and radar_anjalankoski cappi (1 corner)
  const Ref refs[] = {
      {6.6827405, 71.7910205, -196593.004, 8084432.005},
      {15.8230819, 55.9341704, -196593.004, 6255329.699},
      {35.7679564, 56.1308902, 1044176.941, 6255329.699},
      {43.1202918, 72.1975692, 1044176.941, 8084432.005},
      {23.2107739, 62.6482515, 305842.171, 6952104.863},
  };

  for (const auto& r : refs)
  {
    NFmiPoint wxy = area.LatLonToWorldXY(NFmiPoint(r.lon, r.lat));
    NFmiPoint expect(r.easting, r.northing);
    if (expect.Distance(wxy) > 2.0)  // meters
      TEST_FAILED("LatLonToWorldXY(" + tostr(NFmiPoint(r.lon, r.lat)) + ") expected " +
                  tostr(expect) + ", got " + tostr(wxy));
  }
  TEST_PASSED();
}

// ----------------------------------------------------------------------
// Inverse of the same reference points.

void latlon_reference()
{
  NFmiTransverseMercatorArea area = finland();

  struct Ref
  {
    double lon, lat, easting, northing;
  };
  const Ref refs[] = {
      {6.6827405, 71.7910205, -196593.004, 8084432.005},
      {35.7679564, 56.1308902, 1044176.941, 6255329.699},
      {23.2107739, 62.6482515, 305842.171, 6952104.863},
  };

  for (const auto& r : refs)
  {
    NFmiPoint latlon = area.WorldXYToLatLon(NFmiPoint(r.easting, r.northing));
    NFmiPoint expect(r.lon, r.lat);
    if (expect.Distance(latlon) > 1e-4)  // degrees (~10 m); references are rounded
      TEST_FAILED("WorldXYToLatLon(" + tostr(NFmiPoint(r.easting, r.northing)) + ") expected " +
                  tostr(expect) + ", got " + tostr(latlon));
  }
  TEST_PASSED();
}

// ----------------------------------------------------------------------
// Forward/inverse round-trip must be self-consistent to sub-millimeter.

void roundtrip()
{
  NFmiTransverseMercatorArea area = finland();

  const double lons[] = {19, 24, 27, 30, 32};
  const double lats[] = {59, 62, 65, 68, 71};

  for (double lon : lons)
    for (double lat : lats)
    {
      NFmiPoint ll(lon, lat);
      NFmiPoint wxy = area.LatLonToWorldXY(ll);
      NFmiPoint back = area.WorldXYToLatLon(wxy);
      if (ll.Distance(back) > 1e-9)  // degrees
        TEST_FAILED("Round trip failed for " + tostr(ll) + ", got " + tostr(back));
    }
  TEST_PASSED();
}

// ----------------------------------------------------------------------
// Local <-> latlon round-trip (exercises the world<->local scaling too).

void toxy_roundtrip()
{
  NFmiTransverseMercatorArea area = finland();

  NFmiPoint ll(27, 65);
  NFmiPoint xy = area.ToXY(ll);
  NFmiPoint back = area.ToLatLon(xy);
  if (ll.Distance(back) > 1e-9)
    TEST_FAILED("ToXY/ToLatLon round trip failed for " + tostr(ll) + ", got " + tostr(back));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void wkt()
{
  NFmiTransverseMercatorArea area = finland();
  std::string wkt = area.WKT();
  if (wkt.empty())
    TEST_FAILED("WKT is empty");
  if (wkt.find("Mercator") == std::string::npos)
    TEST_FAILED("WKT does not mention Mercator: " + wkt);
  TEST_PASSED();
}

// ----------------------------------------------------------------------
// Write then read back must reproduce an equal area and identical projection.

void serialization()
{
  NFmiTransverseMercatorArea area = finland();

  std::stringstream ss;
  ss << area;

  NFmiTransverseMercatorArea area2;
  ss >> area2;

  if (!(area2 == area))
    TEST_FAILED("Deserialized area differs from the original");

  NFmiPoint ll(27, 65);
  if (area.LatLonToWorldXY(ll).Distance(area2.LatLonToWorldXY(ll)) > 1e-6)
    TEST_FAILED("Deserialized area projects differently");

  if (area2.HashValue() != area.HashValue())
    TEST_FAILED("Deserialized area has a different hash");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void hash_value()
{
  NFmiTransverseMercatorArea a = finland();
  NFmiTransverseMercatorArea b = finland();
  if (a.HashValue() != b.HashValue())
    TEST_FAILED("Identical areas have different hashes");

  // A different central meridian must change the hash.
  NFmiTransverseMercatorArea c(NFmiPoint(19, 59), NFmiPoint(32, 71), 24.0);
  if (a.HashValue() == c.HashValue())
    TEST_FAILED("Areas with different central meridians share a hash");

  // The non-virtual HashValueKludge dispatch (via a base pointer) must reach
  // NFmiTransverseMercatorArea::HashValue.
  const NFmiArea* base = &a;
  if (base->HashValueKludge() != a.HashValue())
    TEST_FAILED("HashValueKludge does not dispatch to NFmiTransverseMercatorArea");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void clone()
{
  NFmiTransverseMercatorArea area = finland();
  NFmiArea* c = area.Clone();
  if (c == nullptr)
    TEST_FAILED("Clone returned null");
  if (!(*c == area))
    TEST_FAILED("Clone differs from the original");
  NFmiPoint ll(27, 65);
  if (area.LatLonToWorldXY(ll).Distance(c->LatLonToWorldXY(ll)) > 1e-6)
    TEST_FAILED("Clone projects differently");
  delete c;
  TEST_PASSED();
}

// ----------------------------------------------------------------------
// NFmiArea::CreateFromBBox with an EPSG:3067 spatial reference must now yield
// a native NFmiTransverseMercatorArea (not NFmiGdalArea).

void createfrombbox()
{
  // EPSG:3067 world rectangle (meters) from a production radar composite.
  NFmiPoint bl(-196593.004, 6255329.699);
  NFmiPoint tr(1044176.941, 8084432.005);

  std::unique_ptr<NFmiArea> area(
      NFmiArea::CreateFromBBox(Fmi::SpatialReference("EPSG:3067"), bl, tr));

  if (area->ClassId() != kNFmiTransverseMercatorArea)
    TEST_FAILED("CreateFromBBox(EPSG:3067) did not yield a native NFmiTransverseMercatorArea");

  // A point on the central meridian must project to easting 500000.
  NFmiPoint wxy = area->LatLonToWorldXY(NFmiPoint(27, 65));
  if (std::fabs(wxy.X() - 500000.0) > 1e-3)
    TEST_FAILED("Routed area projects incorrectly: " + tostr(wxy));

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * The actual test suite
 */
// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  virtual const char* error_message_prefix() const { return "\n\t"; }
  void test(void)
  {
    TEST(create);
    TEST(central_meridian);
    TEST(worldxy_reference);
    TEST(latlon_reference);
    TEST(roundtrip);
    TEST(toxy_roundtrip);
    TEST(wkt);
    TEST(serialization);
    TEST(hash_value);
    TEST(clone);
    TEST(createfrombbox);
  }
};

}  // namespace NFmiTransverseMercatorAreaTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl
       << "NFmiTransverseMercatorArea tester" << endl
       << "=================================" << endl;
  NFmiTransverseMercatorAreaTest::tests t;
  return t.run();
}

// ======================================================================
