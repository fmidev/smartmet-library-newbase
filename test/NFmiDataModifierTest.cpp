// ======================================================================
/*!
 * \file
 * \brief Regression tests for the NFmiDataModifier statistics classes
 */
// ======================================================================

#include "NFmiDataModifierAllValidAvg.h"
#include "NFmiDataModifierAllValidSum.h"
#include "NFmiDataModifierAvg.h"
#include "NFmiDataModifierChange.h"
#include "NFmiDataModifierMax.h"
#include "NFmiDataModifierMedian.h"
#include "NFmiDataModifierMin.h"
#include "NFmiDataModifierMode.h"
#include "NFmiDataModifierStandardDeviation.h"
#include "NFmiDataModifierSum.h"
#include "NFmiGlobals.h"
#include <regression/tframe.h>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

namespace NFmiDataModifierTest
{
const std::vector<float> values{1, 2, 3, 4, 10};
const std::vector<float> values_with_missing{1, kFloatMissing, 2, 3, 4, 10};

float feed(NFmiDataModifier& modifier, const std::vector<float>& theValues)
{
  modifier.Clear();
  for (float value : theValues)
    modifier.Calculate(value);
  return modifier.CalculationResult();
}

std::string str(float value)
{
  return value == kFloatMissing ? "missing" : std::to_string(value);
}

void check(const std::string& name, float result, float expected)
{
  bool ok = (expected == kFloatMissing ? result == kFloatMissing
                                       : std::abs(result - expected) < 1e-4);
  if (!ok)
    TEST_FAILED(name + ": expected " + str(expected) + ", got " + str(result));
}

// ----------------------------------------------------------------------

void basic_statistics()
{
  NFmiDataModifierMin min;
  check("min", feed(min, values), 1);
  check("min with missing", feed(min, values_with_missing), 1);

  NFmiDataModifierMax max;
  check("max", feed(max, values), 10);
  check("max with missing", feed(max, values_with_missing), 10);

  NFmiDataModifierAvg avg;
  check("avg", feed(avg, values), 4);
  check("avg with missing", feed(avg, values_with_missing), 4);

  NFmiDataModifierSum sum;
  check("sum", feed(sum, values), 20);
  check("sum with missing", feed(sum, values_with_missing), 20);

  // Clear resets the state
  check("avg after clear", feed(avg, {5}), 5);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void no_values()
{
  NFmiDataModifierMin min;
  check("min of nothing", feed(min, {}), kFloatMissing);
  NFmiDataModifierMax max;
  check("max of nothing", feed(max, {}), kFloatMissing);
  NFmiDataModifierAvg avg;
  check("avg of nothing", feed(avg, {}), kFloatMissing);
  check("avg of missing", feed(avg, {kFloatMissing, kFloatMissing}), kFloatMissing);
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void median()
{
  NFmiDataModifierMedian median;
  check("median", feed(median, values), 3);
  check("median with missing", feed(median, values_with_missing), 3);

  NFmiDataModifierMedian low(0);
  check("0% fractile", feed(low, values), 1);

  // The 100% fractile used to read past the end of the array
  NFmiDataModifierMedian high(100);
  check("100% fractile", feed(high, values), 10);

  NFmiDataModifierMedian quartile(25);
  check("25% fractile", feed(quartile, values), 2);

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void standard_deviation()
{
  NFmiDataModifierStandardDeviation sdev;
  // Deviations from the mean 4 are -3,-2,-1,0,6, the sum of squares is 50
  // The sample standard deviation divides by n-1
  check("standard deviation", feed(sdev, values), std::sqrt(50.0F / 4));
  check("standard deviation with missing", feed(sdev, values_with_missing), std::sqrt(50.0F / 4));
  check("standard deviation of one value", feed(sdev, {7}), kFloatMissing);
  check("constant standard deviation", feed(sdev, {7, 7, 7}), 0);
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void change()
{
  NFmiDataModifierChange change;
  check("change", feed(change, values), 9);
  check("decreasing change", feed(change, {10, 4, 3}), -7);
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void mode()
{
  NFmiDataModifierMode mode;
  // The most frequent value, not the largest one
  check("mode", feed(mode, {1, 2, 2, 3, 2, 1}), 2);
  check("mode with ties", feed(mode, {5, 4, 4, 5}), 5);
  check("mode with missing", feed(mode, {kFloatMissing, kFloatMissing, 3}), 3);
  TEST_PASSED();
}

// ----------------------------------------------------------------------

void all_valid()
{
  NFmiDataModifierAllValidAvg avg;
  check("all valid avg", feed(avg, values), 4);
  check("all valid avg with missing", feed(avg, values_with_missing), kFloatMissing);

  NFmiDataModifierAllValidSum sum;
  check("all valid sum", feed(sum, values), 20);
  check("all valid sum with missing", feed(sum, values_with_missing), kFloatMissing);
  TEST_PASSED();
}

// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  const char* error_message_prefix() const override { return "\n\t"; }
  void test() override
  {
    TEST(basic_statistics);
    TEST(no_values);
    TEST(median);
    TEST(standard_deviation);
    TEST(change);
    TEST(mode);
    TEST(all_valid);
  }
};

}  // namespace NFmiDataModifierTest

int main()
{
  cout << endl << "NFmiDataModifier tester" << endl << "=======================" << endl;
  NFmiDataModifierTest::tests t;
  return t.run();
}
