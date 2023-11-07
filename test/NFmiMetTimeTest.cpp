// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiMetTime
 */
// ======================================================================

#include "NFmiMetTime.h"
#include <boost/date_time/local_time/local_time.hpp>
#include <regression/tframe.h>

using namespace std;

//! Protection against conflicts with global functions
namespace NFmiMetTimeTest
{
// ----------------------------------------------------------------------

void construct_from_ptime()
{
  using Fmi::DateTime;
  using boost::posix_time::time_from_string;

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 03:00:00");
    NFmiMetTime t(pt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025030000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025030000");
  }

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 04:00:00");
    NFmiMetTime t(pt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025040000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025040000");
  }

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 05:00:00");
    NFmiMetTime t(pt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025050000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025050000");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void construct_from_local_date_time()
{
  boost::local_time::tz_database db;
  db.load_from_file("/usr/share/smartmet/timezones/date_time_zonespec.csv");

  using Fmi::LocalDateTime;
  using boost::local_time::posix_time_zone;
  using Fmi::TimeZonePtr;
  using Fmi::DateTime;
  using boost::posix_time::time_from_string;

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 03:00:00");
    Fmi::TimeZonePtr zone = db.time_zone_from_region("Europe/Stockholm");
    Fmi::LocalDateTime ldt(pt, zone);

    NFmiMetTime t(ldt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025030000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025030000");
  }

  {
    Fmi::DateTime pt = time_from_string("2015-10-25 05:00:00");
    Fmi::TimeZonePtr zone = db.time_zone_from_region("Europe/Helsinki");
    Fmi::LocalDateTime ldt(pt, zone);

    NFmiMetTime t(ldt);
    std::string res = t.ToStr(kYYYYMMDDHHMMSS).CharPtr();
    if (res != "20151025050000")
      TEST_FAILED("Incorrect result " + res + ", expecting 20151025050000");
  }

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void posixtime()
{
  using Fmi::DateTime;
  using boost::posix_time::time_from_string;

  NFmiMetTime t(2013, 10, 25, 1, 2, 3, 0);
  Fmi::DateTime res = t.PosixTime();
  if (res != time_from_string("2013-10-25 01:02:00"))
    TEST_FAILED("Incorrect result " + to_simple_string(res) + " for " +
                t.ToStr(kYYYYMMDDHHMMSS).CharPtr());

  TEST_PASSED();
}

// ----------------------------------------------------------------------

void implicit_conversion()
{
  using Fmi::DateTime;
  using boost::posix_time::time_from_string;

  NFmiMetTime t(2013, 10, 25, 1, 2, 3, 0);
  Fmi::DateTime res = t;
  if (res != time_from_string("2013-10-25 01:02:00"))
    TEST_FAILED("Incorrect result " + to_simple_string(res) + " for " +
                t.ToStr(kYYYYMMDDHHMMSS).CharPtr());

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
    TEST(posixtime);
    TEST(implicit_conversion);
    TEST(construct_from_ptime);
    TEST(construct_from_local_date_time);
  }
};

}  // namespace NFmiMetTimeTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiAngle tester" << endl << "================" << endl;
  NFmiMetTimeTest::tests t;
  return t.run();
}

// ======================================================================
