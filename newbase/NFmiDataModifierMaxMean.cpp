// ======================================================================
/*!
 * \file NFmiDataModifierMaxMean.cpp
 * \brief Implementation of class NFmiDataModifierMaxMean
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMaxMean
 *
 * Undocumented
 *
 * painokerroin (itsWeightFactor) 1 palauttaa keskiarvon, 0 palauttaa maksimin,
 * 0.5 palauttaa (max+avg):n keskiarvon
 * jos painokerrointa ei anneta erikseen, ohjelma palauttaa pistekohtaisesti
 * 'omasta mielestään' kuvaavimman arvon
 *
 */
// ======================================================================

#include "NFmiDataModifierMaxMean.h"

#include <algorithm>

// ----------------------------------------------------------------------
/*!
 * Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxMean::~NFmiDataModifierMaxMean() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxMean::NFmiDataModifierMaxMean()
    : itsCounter(0), itsAverage(0.), itsMaxValue(-3.4E+38F), itsWeightFactor(kFloatMissing)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theWeightFactor Undocumented
 */
// ----------------------------------------------------------------------

NFmiDataModifierMaxMean::NFmiDataModifierMaxMean(float theWeightFactor)
    : itsCounter(0), itsAverage(0.), itsMaxValue(-3.4E+38F), itsWeightFactor(theWeightFactor)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMaxMean::Calculate(float theValue)
{
  if (CheckMissingValues(theValue))
  {
    fCalculationResultOk = true;
    itsCounter++;
    itsAverage -= ((itsAverage - theValue) / itsCounter);

    itsMaxValue = std::max(itsMaxValue, theValue);
  }
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiDataModifierMaxMean::Clear()
{
  NFmiDataModifier::Clear();
  itsMaxValue = -3.4E+38F;
  itsCounter = 0;
  itsAverage = 0;
  fCalculationResultOk = false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

float NFmiDataModifierMaxMean::CalculationResult()
{
  float avg = kFloatMissing;
  float max = kFloatMissing;
  if (fCalculationResultOk)
    avg = itsAverage;
  else
    avg = kFloatMissing;
  if (itsMaxValue != -3.4E+38F)
    max = itsMaxValue;
  else
    max = kFloatMissing;

  if (itsWeightFactor == kFloatMissing && avg != kFloatMissing && max != kFloatMissing)
  {
    float weightFactor = avg / max;
    if (weightFactor < 0.4)
      return avg;
    if (weightFactor >= 0.4 && weightFactor <= 0.6)
      return (avg + max) / 2;
    else if (weightFactor > 0.6 && weightFactor != kFloatMissing)
      return max;
    else
      return kFloatMissing;
  }
  else if (itsWeightFactor != kFloatMissing && avg != kFloatMissing && max != kFloatMissing)
  {
    float result = (avg * itsWeightFactor) + (max * (1 - itsWeightFactor));
    return result;
  }
  else
    return kFloatMissing;
}

// ======================================================================
