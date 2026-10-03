// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiWeatherAndCloudiness
 */
// ======================================================================

#include "NFmiWeatherAndCloudiness.h"
#include <regression/tframe.h>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace NFmiWeatherAndCloudinessTest
{
std::string str(double value)
{
  return std::to_string(value);
}

// Sub parameters with values representable exactly in the packed form

const std::vector<std::pair<FmiParameterName, double>> subvalues{{kFmiTotalCloudCover, 70},
                                                                 {kFmiLowCloudCover, 40},
                                                                 {kFmiMediumCloudCover, 20},
                                                                 {kFmiPrecipitationType, 1},
                                                                 {kFmiPrecipitationForm, 3},
                                                                 {kFmiProbabilityThunderstorm, 30},
                                                                 {kFmiFogIntensity, 2}};

// ----------------------------------------------------------------------

void subvalues_roundtrip()
{
  NFmiWeatherAndCloudiness weather(7.);
  for (const auto& item : subvalues)
    if (!weather.SubValue(item.second, item.first))
      TEST_FAILED("Failed to set parameter " + std::to_string(item.first));

  // Setting one value must not change the others
  for (const auto& item : subvalues)
  {
    double value = weather.SubValue(item.first);
    if (std::abs(value - item.second) > 1e-6)
      TEST_FAILED("Parameter " + std::to_string(item.first) + " should be " + str(item.second) +
                  ", got " + str(value));
  }

  // The values must survive packing
  NFmiWeatherAndCloudiness copy(weather.LongValue(), kFmiPackedWeather, kFloatMissing, 7.);
  for (const auto& item : subvalues)
  {
    double value = copy.SubValue(item.first);
    if (std::abs(value - item.second) > 1e-6)
      TEST_FAILED("Packed parameter " + std::to_string(item.first) + " should be " +
                  str(item.second) + ", got " + str(value));
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void cloud_cover_resolution()
{
  // Cloud covers are stored with a 10% resolution
  NFmiWeatherAndCloudiness weather(7.);
  weather.SubValue(64, kFmiTotalCloudCover);
  if (weather.SubValue(kFmiTotalCloudCover) != 60)
    TEST_FAILED("64% should be stored as 60%, got " + str(weather.SubValue(kFmiTotalCloudCover)));
  weather.SubValue(100, kFmiTotalCloudCover);
  if (weather.SubValue(kFmiTotalCloudCover) != 100)
    TEST_FAILED("100% should be stored as 100%, got " +
                str(weather.SubValue(kFmiTotalCloudCover)));
  weather.SubValue(-5, kFmiTotalCloudCover);
  if (weather.SubValue(kFmiTotalCloudCover) != 0)
    TEST_FAILED("Negative cloud cover should be stored as 0%, got " +
                str(weather.SubValue(kFmiTotalCloudCover)));
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void precipitation()
{
  // Precipitation is stored with a non-linear scale, small amounts more accurately
  for (double rr : {0.0, 0.1, 0.4, 1.0, 2.5, 7.0, 20.0})
  {
    NFmiWeatherAndCloudiness weather(7.);
    weather.SubValue(rr, kFmiPrecipitation1h);
    double value = weather.SubValue(kFmiPrecipitation1h);
    double tolerance = std::max(0.05, 0.1 * rr);
    if (std::abs(value - rr) > tolerance)
      TEST_FAILED("Precipitation " + str(rr) + " came back as " + str(value));

    NFmiWeatherAndCloudiness copy(weather.LongValue(), kFmiPackedWeather, kFloatMissing, 7.);
    if (copy.SubValue(kFmiPrecipitation1h) != value)
      TEST_FAILED("Packed precipitation " + str(rr) + " changed to " +
                  str(copy.SubValue(kFmiPrecipitation1h)));
  }

  // Increasing amounts must stay in order
  double previous = -1;
  for (double rr = 0; rr < 30; rr += 0.3)
  {
    NFmiWeatherAndCloudiness weather(7.);
    weather.SubValue(rr, kFmiPrecipitation1h);
    double value = weather.SubValue(kFmiPrecipitation1h);
    if (value < previous)
      TEST_FAILED("Precipitation " + str(rr) + " gave " + str(value) + " < " + str(previous));
    previous = value;
  }
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void missing_values()
{
  // A new object has no values
  NFmiWeatherAndCloudiness empty(7.);
  for (const auto& item : subvalues)
    if (empty.SubValue(item.first) != kFloatMissing)
      TEST_FAILED("Parameter " + std::to_string(item.first) + " should initially be missing, got " +
                  str(empty.SubValue(item.first)));

  // The setters do not accept missing values: the old value is kept and false is returned
  NFmiWeatherAndCloudiness weather(7.);
  for (const auto& item : subvalues)
  {
    weather.SubValue(item.second, item.first);
    if (weather.SubValue(kFloatMissing, item.first))
      TEST_FAILED("Setting parameter " + std::to_string(item.first) +
                  " to missing should be rejected");
    if (std::abs(weather.SubValue(item.first) - item.second) > 1e-6)
      TEST_FAILED("Parameter " + std::to_string(item.first) + " should have kept its value");
  }
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void precipitation_form_classes()
{
  // 0=drizzle 1=rain 2=sleet 3=snow 4=freezing drizzle 5=freezing rain 6=hail
  if (!NFmiWeatherAndCloudiness::IsLiquid(1) || NFmiWeatherAndCloudiness::IsLiquid(3))
    TEST_FAILED("Rain is liquid, snow is not");
  if (!NFmiWeatherAndCloudiness::IsSolid(3) || NFmiWeatherAndCloudiness::IsSolid(1))
    TEST_FAILED("Snow is solid, rain is not");
  if (!NFmiWeatherAndCloudiness::IsHalfLiquid(2))
    TEST_FAILED("Sleet is half liquid");
  TEST_PASSED();
}

// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  const char* error_message_prefix() const override { return "\n\t"; }
  void test() override
  {
    TEST(subvalues_roundtrip);
    TEST(cloud_cover_resolution);
    TEST(precipitation);
    TEST(missing_values);
    TEST(precipitation_form_classes);
  }
};

}  // namespace NFmiWeatherAndCloudinessTest

int main()
{
  cout << endl << "NFmiWeatherAndCloudiness tester" << endl << "===============================" << endl;
  NFmiWeatherAndCloudinessTest::tests t;
  return t.run();
}
