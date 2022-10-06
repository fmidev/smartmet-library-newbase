// ======================================================================
/*!
 * \file
 * \brief Regression tests for class NFmiAreaFactory
 */
// ======================================================================

#include "NFmiArea.h"
#include "NFmiAreaFactory.h"
#include <regression/tframe.h>

//! Protection against conflicts with global functions
namespace NFmiAreaFactoryTest
{
void latlon()
{
  std::string proj = "latlon:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

  TEST_PASSED();
}

void invrotlatlon()
{
  std::string proj = "invrotlatlon,-90,30:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

  TEST_PASSED();
}

void mercator()
{
  std::string proj = "mercator:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

  TEST_PASSED();
}

void ykj()
{
  std::string proj = "ykj:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

  TEST_PASSED();
}

void stereographic()
{
  std::string proj = "stereographic,25,90,60:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

  TEST_PASSED();
}

void lcc()
{
  std::string proj = "lcc,25,60,50,70:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

  TEST_PASSED();
}

void equidist()
{
  std::string proj = "equidist,25,60:10,20,30,40";
  auto area = NFmiAreaFactory::Create(proj);
  auto str = area->AreaFactoryStr();

  if (str != proj)
    TEST_FAILED("Output " + str + " != expected " + proj);

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
    TEST(latlon);
    TEST(invrotlatlon);
    TEST(mercator);
    TEST(ykj);
    TEST(stereographic);
    TEST(lcc);
    TEST(equidist);
  }
};

}  // namespace NFmiAreaFactoryTest

//! The main program
int main(void)
{
  using namespace std;
  cout << endl << "NFmiAreaFactory tester" << endl << "======================" << endl;
  NFmiAreaFactoryTest::tests t;
  return t.run();
}

// ======================================================================
