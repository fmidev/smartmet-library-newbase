// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiTotalWind
 */
// ======================================================================

#include "NFmiTotalWind.h"
#include <regression/tframe.h>
#include <cmath>
#include <string>

using namespace std;

namespace NFmiTotalWindTest
{
bool close(double value, double expected, double tolerance)
{
  return std::abs(value - expected) <= tolerance;
}

std::string str(double value)
{
  return std::to_string(value);
}

// ----------------------------------------------------------------------

void direction_and_speed()
{
  NFmiTotalWind wind(270, 10, kFmiDirectionAndSpeed);
  if (!close(wind.SubValue(kFmiWindDirection), 270, 0.5))
    TEST_FAILED("Direction should be 270, got " + str(wind.SubValue(kFmiWindDirection)));
  if (!close(wind.SubValue(kFmiWindSpeedMS), 10, 0.05))
    TEST_FAILED("Speed should be 10, got " + str(wind.SubValue(kFmiWindSpeedMS)));

  // Westerly wind blows towards the east
  if (!close(wind.SubValue(kFmiWindUMS), 10, 0.05))
    TEST_FAILED("U should be 10, got " + str(wind.SubValue(kFmiWindUMS)));
  if (!close(wind.SubValue(kFmiWindVMS), 0, 0.05))
    TEST_FAILED("V should be 0, got " + str(wind.SubValue(kFmiWindVMS)));

  // Northerly wind blows towards the south
  NFmiTotalWind north(0, 5, kFmiDirectionAndSpeed);
  if (!close(north.SubValue(kFmiWindUMS), 0, 0.05))
    TEST_FAILED("U should be 0, got " + str(north.SubValue(kFmiWindUMS)));
  if (!close(north.SubValue(kFmiWindVMS), -5, 0.05))
    TEST_FAILED("V should be -5, got " + str(north.SubValue(kFmiWindVMS)));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void uv_components()
{
  // Wind towards the north is a southerly wind
  NFmiTotalWind wind(0, 8, kFmiUVComponents);
  if (!close(wind.SubValue(kFmiWindDirection), 180, 0.5))
    TEST_FAILED("Direction should be 180, got " + str(wind.SubValue(kFmiWindDirection)));
  if (!close(wind.SubValue(kFmiWindSpeedMS), 8, 0.05))
    TEST_FAILED("Speed should be 8, got " + str(wind.SubValue(kFmiWindSpeedMS)));

  // Diagonal
  NFmiTotalWind diagonal(3, 4, kFmiUVComponents);
  if (!close(diagonal.SubValue(kFmiWindSpeedMS), 5, 0.05))
    TEST_FAILED("Speed should be 5, got " + str(diagonal.SubValue(kFmiWindSpeedMS)));
  // 216.9 degrees is stored with the 10 degree resolution
  if (!close(diagonal.SubValue(kFmiWindDirection), 220, 0.5))
    TEST_FAILED("Direction should be 220, got " + str(diagonal.SubValue(kFmiWindDirection)));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void direction_resolution()
{
  // The direction is stored with a 10 degree resolution
  NFmiTotalWind wind(123, 5, kFmiDirectionAndSpeed);
  if (!close(wind.SubValue(kFmiWindDirection), 120, 0.5))
    TEST_FAILED("Direction 123 should be stored as 120, got " +
                str(wind.SubValue(kFmiWindDirection)));

  // North is 360 when there is wind, 0 when calm
  NFmiTotalWind north(0, 5, kFmiDirectionAndSpeed);
  if (!close(north.SubValue(kFmiWindDirection), 360, 0.5))
    TEST_FAILED("Northerly wind direction should be 360, got " +
                str(north.SubValue(kFmiWindDirection)));
  NFmiTotalWind calm(0, 0, kFmiDirectionAndSpeed);
  if (!close(calm.SubValue(kFmiWindDirection), 0, 0.5))
    TEST_FAILED("Calm wind direction should be 0, got " + str(calm.SubValue(kFmiWindDirection)));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void packing()
{
  // The packed value must give back the same components
  for (double dir : {0.0, 45.0, 90.0, 123.0, 180.0, 359.0})
    for (double speed : {0.0, 0.5, 3.2, 12.7, 35.0})
    {
      NFmiTotalWind wind(dir, speed, kFmiDirectionAndSpeed, 7., 20);
      NFmiTotalWind copy(wind.LongValue(), kFmiPackedWind, 7.);
      const std::string where = "dir=" + str(dir) + " speed=" + str(speed);
      if (!close(copy.SubValue(kFmiWindDirection), wind.SubValue(kFmiWindDirection), 0.5))
        TEST_FAILED("Direction changed in packing: " + where);
      if (!close(copy.SubValue(kFmiWindSpeedMS), wind.SubValue(kFmiWindSpeedMS), 0.05))
        TEST_FAILED("Speed changed in packing: " + where);
      if (!close(copy.SubValue(kFmiHourlyMaximumGust), 20, 0.05))
        TEST_FAILED("Gust changed in packing: " + where + " got " +
                    str(copy.SubValue(kFmiHourlyMaximumGust)));
    }
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void missing_values()
{
  NFmiTotalWind wind(kFloatMissing, kFloatMissing, kFmiDirectionAndSpeed);
  if (wind.SubValue(kFmiWindSpeedMS) != kFloatMissing)
    TEST_FAILED("Missing speed should stay missing, got " + str(wind.SubValue(kFmiWindSpeedMS)));
  if (wind.SubValue(kFmiWindUMS) != kFloatMissing)
    TEST_FAILED("U of missing wind should be missing, got " + str(wind.SubValue(kFmiWindUMS)));

  NFmiTotalWind nogust(90, 5, kFmiDirectionAndSpeed);
  if (nogust.SubValue(kFmiHourlyMaximumGust) != kFloatMissing)
    TEST_FAILED("Gust should be missing when not given, got " +
                str(nogust.SubValue(kFmiHourlyMaximumGust)));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  const char* error_message_prefix() const override { return "\n\t"; }
  void test() override
  {
    TEST(direction_and_speed);
    TEST(uv_components);
    TEST(direction_resolution);
    TEST(packing);
    TEST(missing_values);
  }
};

}  // namespace NFmiTotalWindTest

int main()
{
  cout << endl << "NFmiTotalWind tester" << endl << "====================" << endl;
  NFmiTotalWindTest::tests t;
  return t.run();
}
