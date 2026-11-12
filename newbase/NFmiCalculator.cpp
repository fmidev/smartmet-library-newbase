// ======================================================================
/*!
 * \file NFmiCalculator.cpp
 * \brief Implementation of class NFmiCalculator
 */
// ======================================================================
/*!
 * \class NFmiCalculator
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiCalculator.h"
#include "NFmiDataIterator.h"
#include "NFmiDataModifier.h"
#include "NFmiGlobals.h"
#include <macgyver/Exception.h>
#include <string>

class NFmiCombinedParam;
class NFmiQueryInfo;

// ----------------------------------------------------------------------.
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiCalculator::~NFmiCalculator() = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theDataIterator Undocumented
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculator::NFmiCalculator(NFmiDataIterator* theDataIterator, NFmiDataModifier* theDataModifier)
    : itsDataModifier(theDataModifier), itsDataIterator(theDataIterator), itsData(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theData Undocumented
 * \param theDataModifier Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculator::NFmiCalculator(NFmiQueryInfo* theData, NFmiDataModifier* theDataModifier)
    : itsDataModifier(theDataModifier), itsDataIterator(nullptr), itsData(theData)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiCalculator::FloatValue()
{
  try
  {
    if (itsDataIterator && itsDataModifier)
    {
      itsDataIterator->DoForEach(itsDataModifier);
      return itsDataModifier->CalculationResult();
    }
    if (itsData && itsDataModifier)
    {
      return itsDataModifier->FloatValue();
    }
    return kFloatMissing;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiCombinedParam* NFmiCalculator::CombinedValue()
{
  try
  {
    if (itsDataIterator && itsDataModifier)
    {
      itsDataIterator->DoForEach(itsDataModifier);
      return itsDataModifier->CombinedCalculationResult();
    }
    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifier* NFmiCalculator::CalculatedModifier()
{
  try
  {
    if (itsDataIterator && itsDataModifier)
    {
      itsDataIterator->DoForEach(itsDataModifier);
      return itsDataModifier;
    }
    return nullptr;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// Tämä on kuten FloatValue, paitsi että juoksutus tehdään täällä loopissa
// ja laskut tehdään täällä eikä iteratorin DoForEach-metodissa.

float NFmiCalculator::CalculatedValue()
{
  return kFloatMissing;
}
// ======================================================================
