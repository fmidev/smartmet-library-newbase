// ======================================================================
/*!
 * \file NFmiDataModifierMinPlace.cpp
 * \brief Implementation of class NFmiDataModifierMinPlace
 */
// ======================================================================

#include "NFmiDataModifierMinPlace.h"

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMinPlace::~NFmiDataModifierMinPlace() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMinPlace::NFmiDataModifierMinPlace()
{
  itsExtremeValue = 3.4E+38F;
}
// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------
/*
void NFmiDataModifierMaxPlace::Calculate(float theValue)
{
  if(theValue != kFloatMissing)
        if(itsExtremeValue < theValue)
          itsExtremeValue = theValue;
}
*/
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMinPlace::Clear()
{
  itsExtremeValue = 3.4E+38F;
}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMinPlace::CalculationResult()
{
  return itsExtremeValue != 3.4E+38F && (fMissingValuesAllowed || itsNumberOfMissingValues <= 0)
             ? itsExtremeValue
             : kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param value Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiDataModifierMinPlace::IsNewExtreme(float value)
{
  return value < itsExtremeValue;
}
// ======================================================================
