// ======================================================================
/*!
 * \file NFmiDataModifierVariance.cpp
 * \brief Implementation of class NFmiDataModifierVariance
 */
// ======================================================================
/*!
 * \class NFmiDataModifierVariance
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiDataModifierVariance.h"

#include "NFmiQueryInfo.h"
#include "NFmiRegressionItem.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

using WriteLock = boost::unique_lock<MutexType>;

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierVariance::~NFmiDataModifierVariance() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theDataIdent Undocumented
 * \param theLevel Undocumented
 * \param theData Undocumented
 * \param theVarianceLevel Undocumented
 * \param onlyVariance Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierVariance::NFmiDataModifierVariance(NFmiDataIdent* theDataIdent,
                                                   NFmiLevel* theLevel,
                                                   NFmiQueryInfo* theData,
                                                   float theVarianceLevel,
                                                   bool onlyVariance)
    : NFmiRegressionModifier(theDataIdent, theLevel, theData),
      fVarianceOnly(onlyVariance),
      itsVarianceLevel(theVarianceLevel)
{
  // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp): the seed is not used for security
  srand(static_cast<unsigned int>(time(nullptr)));
  // AddSubModifier(&NFmiRegressionItem);//Vain testiä varten
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataModifierVariance::FloatValue()
{
  double variance = NFmiRegressionModifier::FloatValue();
  if (variance == kFloatMissing)
    return variance;
  double raw = NAN;
  if (!fVarianceOnly)
  {
    raw = itsData->FloatValue();
    if (raw == kFloatMissing)
      return raw;

    return raw + itsVarianceLevel * GaussianRandom() * variance;
  }

  return itsVarianceLevel * GaussianRandom() * variance;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiDataModifierVariance::GaussianRandom()
{
  // Using static local variables like this is not
  // thread safe - we need to lock. Note also that
  // calling rand() is not thread safe. This should
  // be rewritten using the boost math library.

  WriteLock lock(itsMutex);

  static double V1;
  static double V2;
  static double S;
  static int phase = 0;
  double X = NAN;

  if (phase == 0)
  {
    for (bool more = true; more; more = S >= 1 || S == 0)
    {
      // NOLINTNEXTLINE(cert-msc50-cpp): rand() is not used for security here
      double U1 = static_cast<double>(rand()) / RAND_MAX;
      // NOLINTNEXTLINE(cert-msc50-cpp): rand() is not used for security here
      double U2 = static_cast<double>(rand()) / RAND_MAX;
      V1 = 2 * U1 - 1;
      V2 = 2 * U2 - 1;
      S = V1 * V1 + V2 * V2;
    }
    X = V1 * sqrt(-2 * log(S) / S);
  }
  else
    X = V2 * sqrt(-2 * log(S) / S);
  phase = 1 - phase;
  return X;
}

// ======================================================================
