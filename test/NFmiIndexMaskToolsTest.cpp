// ======================================================================
/*!
 * \file
 * \brief Regression tests for namespace NFmiIndexMaskTools
 */
// ======================================================================

#include "NFmiAreaFactory.h"
#include "NFmiFileSystem.h"
#include "NFmiGrid.h"
#include "NFmiIndexMask.h"
#include "NFmiIndexMaskTools.h"
#include "NFmiStreamQueryData.h"
#include "NFmiSvgPath.h"
#include "NFmiSvgTools.h"
#include <regression/tframe.h>
#include <fstream>
#include <sstream>

//! Protection against conflicts with global functions
namespace NFmiIndexMaskToolsTest
{
NFmiStreamQueryData theQD;
const NFmiGrid* theGrid;

NFmiSvgPath theSuomi;
NFmiSvgPath theCoast;

void read_querydata(const std::string& theFilename)
{
  theQD.ReadData(theFilename);
  theGrid = theQD.QueryInfoIter()->Grid();
}

void read_svg()
{
  std::ifstream insuomi("data/suomi.svg", std::ios::in);
  insuomi >> theSuomi;
  insuomi.close();

  std::ifstream incoast("data/vesiraja.svg", std::ios::in);
  incoast >> theCoast;
  incoast.close();
}

std::string maskdifference(const std::string& theLhs, const std::string& theRhs)
{
  std::string ret;
  std::string::const_iterator it1 = theLhs.begin();
  std::string::const_iterator it2 = theRhs.begin();
  for (; it1 != theLhs.end() && it2 != theRhs.end(); ++it1, ++it2)
  {
    if (*it1 == *it2)
      ret += *it1;
    else if (*it1 == '.')
      ret += '+';
    else
      ret += '-';
  }
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskInside()
 */
// ----------------------------------------------------------------------

void maskinside(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskInside(*theGrid, theSuomi);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_inside.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskInside failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskOutside()
 */
// ----------------------------------------------------------------------

void maskoutside(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskOutside(*theGrid, theSuomi);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_outside.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskOutside failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskExpand()
 */
// ----------------------------------------------------------------------

void maskexpand(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskExpand(*theGrid, theSuomi, 50);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_expand.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
    cout << endl << "Difference = " << endl << maskdifference(expected, result) << endl;
    TEST_FAILED("MaskExpand failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskShrink()
 */
// ----------------------------------------------------------------------

void maskshrink(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiIndexMask suomi = MaskShrink(*theGrid, theSuomi, 25);
  string expected;
  NFmiFileSystem::ReadFile2String("data/suomi_shrink.mask", expected);
  string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskShrink failed for suomi.svg");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskDistance(path)
 */
// ----------------------------------------------------------------------

void maskdistancepath(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  {
    NFmiIndexMask suomi = MaskDistance(*theGrid, theSuomi, 25);
    string expected;
    NFmiFileSystem::ReadFile2String("data/suomi_distance.mask", expected);
    string result = MaskString(suomi, theGrid->XNumber(), theGrid->YNumber());
    if (result != expected)
    {
#if 0
		  cout << endl
			   << "Difference = " << endl
			   << maskdifference(expected,result) << endl;
#endif
      TEST_FAILED("MaskDistance failed for suomi.svg");
    }
  }

  {
    NFmiIndexMask coast = MaskDistance(*theGrid, theCoast, 25);
    string expected;
    NFmiFileSystem::ReadFile2String("data/coast_distance.mask", expected);
    string result = MaskString(coast, theGrid->XNumber(), theGrid->YNumber());
    if (result != expected)
    {
#if 0
		  cout << endl
			   << "Difference = " << endl
			   << maskdifference(expected,result) << endl;
#endif
      TEST_FAILED("MaskDistance failed for coast.svg");
    }
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskDistance(point)
 */
// ----------------------------------------------------------------------

void maskdistancepoint(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  NFmiPoint hki(25, 60);
  NFmiIndexMask helsinki = MaskDistance(*theGrid, hki, 100);
  string expected;
  NFmiFileSystem::ReadFile2String("data/helsinki_distance.mask", expected);
  string result = MaskString(helsinki, theGrid->XNumber(), theGrid->YNumber());
  if (result != expected)
  {
#if 0
		cout << endl
			 << "Difference = " << endl
			 << maskdifference(expected,result) << endl;
#endif
    TEST_FAILED("MaskDistance failed for helsinki");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskExpand() for a vector of distances
 */
// ----------------------------------------------------------------------

void maskexpandmany(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  std::vector<double> distances;
  distances.push_back(0);
  distances.push_back(-25);
  distances.push_back(50);

  string expected1, expected2, expected3;
  NFmiFileSystem::ReadFile2String("data/suomi_inside.mask", expected1);
  NFmiFileSystem::ReadFile2String("data/suomi_shrink.mask", expected2);
  NFmiFileSystem::ReadFile2String("data/suomi_expand.mask", expected3);

  std::vector<NFmiIndexMask> masks;
  masks = MaskExpand(*theGrid, theSuomi, distances);

  if (masks.size() != 3)
    TEST_FAILED("MaskExpand failed to return vector of 3 masks");

  string result1 = MaskString(masks[0], theGrid->XNumber(), theGrid->YNumber());
  string result2 = MaskString(masks[1], theGrid->XNumber(), theGrid->YNumber());
  string result3 = MaskString(masks[2], theGrid->XNumber(), theGrid->YNumber());

  if (result1 != expected1)
  {
    cout << endl
         << "Difference for distance 0 = " << endl
         << maskdifference(expected1, result1) << endl;
    TEST_FAILED("MaskExpand failed for distance 0 in vector");
  }
  if (result2 != expected2)
  {
    cout << endl
         << "Difference for distance -25 = " << endl
         << maskdifference(expected2, result2) << endl;
    TEST_FAILED("MaskExpand failed for distance -25 in vector");
  }
  if (result3 != expected3)
  {
    cout << endl
         << "Difference for distance 50 = " << endl
         << maskdifference(expected3, result3) << endl;
    TEST_FAILED("MaskExpand failed for distance 50 in vector");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------
/*!
 * \brief Test MaskInside() and MaskOutside() with large polygons
 *
 * The masks must agree with a brute force insidedness test of every grid
 * point also when the polygon is much larger than the grid, when its center
 * is outside the grid and when the projection bends its edges strongly
 * (rotated latlon).
 */
// ----------------------------------------------------------------------

void masklargepolygons(void)
{
  using namespace std;
  using namespace NFmiIndexMaskTools;

  // A rotated latlon area like that of the HARMONIE test data in smartmet-test-data
  auto rotated = NFmiAreaFactory::Create("rotlatlon,-30,20:24.908,59.9827,26.0874,61.0171");
  NFmiGrid rotatedgrid(rotated.get(), 15, 20);

  const vector<pair<string, const NFmiGrid*>> grids{{"hiladata", theGrid},
                                                    {"rotlatlon", &rotatedgrid}};

  const vector<string> paths{"\"M -50 -50 L -50 70 L 70 70 L 70 -50 Z\"",
                             "\"M 20 60 L 20 65 L 120 65 L 120 60 Z\"",
                             "\"M 10 60.3 L 10 70 L 179 70 L 179 60.3 Z\"",
                             "\"M 24 59 L 24 62 L 27 62 L 27 59 Z\"",
                             "\"M 25.5 60.5 L 25.5 61.5 L 27 61.5 L 27 60.5 Z\""};

  for (const auto& grid : grids)
  {
    const unsigned long n = grid.second->XNumber() * grid.second->YNumber();

    for (const auto& svg : paths)
    {
      NFmiSvgPath path;
      istringstream in(svg);
      in >> path;

      NFmiIndexMask expected;
      for (unsigned long idx = 0; idx < n; idx++)
        if (NFmiSvgTools::IsInside(path, grid.second->LatLon(idx)))
          expected.insert(idx);

      const NFmiIndexMask inside = MaskInside(*grid.second, path);
      if (inside != expected)
        TEST_FAILED("MaskInside failed for " + svg + " in " + grid.first + " grid: " +
                    to_string(inside.size()) + " points instead of " +
                    to_string(expected.size()));

      const NFmiIndexMask outside = MaskOutside(*grid.second, path);
      if (outside.size() + expected.size() != n)
        TEST_FAILED("MaskOutside failed for " + svg + " in " + grid.first + " grid: " +
                    to_string(outside.size()) + " points instead of " +
                    to_string(n - expected.size()));
    }
  }

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
    TEST(maskinside);
    TEST(maskoutside);
    TEST(maskexpand);
    TEST(maskshrink);
    TEST(maskdistancepath);
    TEST(maskdistancepoint);
    TEST(maskexpandmany);
    TEST(masklargepolygons);
  }
};

}  // namespace NFmiIndexMaskToolsTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiIndexMaskTools tester" << endl << "=========================" << endl;

  NFmiIndexMaskToolsTest::read_querydata("data/hiladata.sqd");
  NFmiIndexMaskToolsTest::read_svg();

  NFmiIndexMaskToolsTest::tests t;
  return t.run();
}

// ======================================================================
