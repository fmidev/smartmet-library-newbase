// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiTimeBag
 */
// ======================================================================

#include "NFmiTimeBag.h"
#include <regression/tframe.h>
#include <string>

using namespace std;

namespace NFmiTimeBagTest
{
NFmiMetTime t(int day, int hour, int minute = 0)
{
  return NFmiMetTime(2008, 8, day, hour, minute, 0, 1);
}

std::string str(const NFmiMetTime& time)
{
  return std::string(time.ToStr(kYYYYMMDDHHMM).CharPtr());
}

// 2008-08-05 00:00 ... 2008-08-06 00:00 every 3 hours = 9 times
NFmiTimeBag bag()
{
  return NFmiTimeBag(t(5, 0), t(6, 0), NFmiTimePerioid(180));
}

// ----------------------------------------------------------------------

void size_and_iteration()
{
  auto b = bag();
  if (b.GetSize() != 9)
    TEST_FAILED("Size should be 9, got " + std::to_string(b.GetSize()));
  if (b.IsEmpty())
    TEST_FAILED("Bag should not be empty");

  int count = 0;
  NFmiMetTime previous;
  for (b.Reset(); b.Next();)
  {
    if (count > 0 && b.CurrentTime().DifferenceInMinutes(previous) != 180)
      TEST_FAILED("Times should be 180 minutes apart at " + str(b.CurrentTime()));
    previous = b.CurrentTime();
    ++count;
  }
  if (count != 9)
    TEST_FAILED("Iteration should visit 9 times, got " + std::to_string(count));
  if (previous != t(6, 0))
    TEST_FAILED("Last iterated time should be the last time, got " + str(previous));

  // Backward iteration
  count = 0;
  for (b.Reset(kBackward); b.Previous();)
  {
    if (count == 0 && b.CurrentTime() != t(6, 0))
      TEST_FAILED("Backward iteration should start from the last time, got " +
                  str(b.CurrentTime()));
    ++count;
  }
  if (count != 9)
    TEST_FAILED("Backward iteration should visit 9 times, got " + std::to_string(count));

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void inside_and_current()
{
  auto b = bag();
  if (!b.IsInside(t(5, 4)))
    TEST_FAILED("05 04:00 should be inside");
  if (!b.IsInside(t(5, 0)) || !b.IsInside(t(6, 0)))
    TEST_FAILED("The end points should be inside");
  if (b.IsInside(t(6, 3)))
    TEST_FAILED("06 03:00 should not be inside");

  if (!b.SetCurrent(t(5, 9)))
    TEST_FAILED("Setting an existing time should succeed");
  if (b.CurrentTime() != t(5, 9))
    TEST_FAILED("Current time should be 05 09:00, got " + str(b.CurrentTime()));
  if (b.SetCurrent(t(5, 10)))
    TEST_FAILED("Setting a time between the timesteps should fail");

  if (!b.SetTime(2) || b.CurrentTime() != t(5, 6))
    TEST_FAILED("Time index 2 should be 05 06:00, got " + str(b.CurrentTime()));
  if (b.SetTime(9))
    TEST_FAILED("Time index 9 should be out of range");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void nearest_time()
{
  auto b = bag();

  if (!b.FindNearestTime(t(5, 4)) || b.CurrentTime() != t(5, 3))
    TEST_FAILED("Nearest time to 04:00 should be 03:00, got " + str(b.CurrentTime()));
  if (!b.FindNearestTime(t(5, 5)) || b.CurrentTime() != t(5, 6))
    TEST_FAILED("Nearest time to 05:00 should be 06:00, got " + str(b.CurrentTime()));

  if (!b.FindNearestTime(t(5, 5), kBackward) || b.CurrentTime() != t(5, 3))
    TEST_FAILED("Nearest earlier time to 05:00 should be 03:00, got " + str(b.CurrentTime()));
  if (!b.FindNearestTime(t(5, 4), kForward) || b.CurrentTime() != t(5, 6))
    TEST_FAILED("Nearest later time to 04:00 should be 06:00, got " + str(b.CurrentTime()));

  // A time range limit
  if (b.FindNearestTime(t(5, 4, 30), kCenter, 60))
    TEST_FAILED("No time should be within 60 minutes of 04:30");
  if (!b.FindNearestTime(t(5, 4, 30), kCenter, 90))
    TEST_FAILED("03:00 and 06:00 are within 90 minutes of 04:30");

  // Outside the bag
  if (!b.FindNearestTime(t(7, 0)) || b.CurrentTime() != t(6, 0))
    TEST_FAILED("Nearest time after the bag should be the last time, got " +
                str(b.CurrentTime()));
  if (b.FindNearestTime(t(7, 0), kForward))
    TEST_FAILED("No later time should exist after the bag");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void intersection_and_prune()
{
  auto b = bag();
  NFmiTimeBag other(t(5, 12), t(6, 12), NFmiTimePerioid(180));
  NFmiTimeBag result;
  if (!b.CalcIntersection(other, result))
    TEST_FAILED("The bags should intersect");
  if (result.FirstTime() != t(5, 12) || result.LastTime() != t(6, 0))
    TEST_FAILED("Intersection should be 05 12:00-06 00:00, got " + str(result.FirstTime()) +
                "-" + str(result.LastTime()));

  NFmiTimeBag disjoint(t(7, 0), t(7, 12), NFmiTimePerioid(180));
  if (b.CalcIntersection(disjoint, result))
    TEST_FAILED("Disjoint bags should not intersect");

  auto pruned = bag();
  pruned.PruneTimes(4);
  if (pruned.GetSize() != 4 || pruned.FirstTime() != t(5, 0))
    TEST_FAILED("Pruning from the end should keep the 4 first times, got " +
                std::to_string(pruned.GetSize()) + " from " + str(pruned.FirstTime()));

  auto pruned2 = bag();
  pruned2.PruneTimes(4, false);
  if (pruned2.GetSize() != 4 || pruned2.LastTime() != t(6, 0))
    TEST_FAILED("Pruning from the start should keep the 4 last times, got " +
                std::to_string(pruned2.GetSize()) + " until " + str(pruned2.LastTime()));

  auto moved = bag();
  moved.MoveByMinutes(60);
  if (moved.FirstTime() != t(5, 1) || moved.LastTime() != t(6, 1) || moved.GetSize() != 9)
    TEST_FAILED("Moving by 60 minutes should shift the whole bag");

  TEST_PASSED();
}

// ----------------------------------------------------------------------

class tests : public tframe::tests
{
  const char* error_message_prefix() const override { return "\n\t"; }
  void test() override
  {
    TEST(size_and_iteration);
    TEST(inside_and_current);
    TEST(nearest_time);
    TEST(intersection_and_prune);
  }
};

}  // namespace NFmiTimeBagTest

int main()
{
  cout << endl << "NFmiTimeBag tester" << endl << "==================" << endl;
  NFmiTimeBagTest::tests t;
  return t.run();
}
