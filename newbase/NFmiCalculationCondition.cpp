// ======================================================================
/*!
 * \file NFmiCalculationCondition.cpp
 * \brief Implementation of class NFmiCalculationCondition
 */
// ======================================================================
/*!
 * \class NFmiCalculationCondition
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiCalculationCondition.h"
#include "NFmiValueString.h"
#include <macgyver/Exception.h>
#include <cmath>
#include <iostream>

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theCondition Undocumented
 * \param theLowerLimit Undocumented
 * \param theUpperLimit Undocumented
 */
// ----------------------------------------------------------------------

NFmiCalculationCondition::NFmiCalculationCondition(FmiMaskOperation theCondition,
                                                   double theLowerLimit,
                                                   double theUpperLimit)
    : itsCondition(theCondition), itsUpperLimit(theUpperLimit), itsLowerLimit(theLowerLimit)
{
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiCalculationCondition::NFmiCalculationCondition()
    : itsCondition(kFmiNoMaskOperation), itsUpperLimit(1), itsLowerLimit(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiCalculationCondition::IsMasked(double theValue) const
{
  try
  {
    if (itsCondition == kFmiNoMaskOperation)
      return true;
    if (theValue == kFloatMissing)
      return false;
    else
    {
      switch (itsCondition)
      {
        case kFmiNoMaskOperation:
          return true;
        case kFmiMaskEqual:
          return (round(theValue) == itsLowerLimit);  // kuinka suurella tarkkuudella???
        case kFmiMaskNotEqual:
          return (round(theValue) != itsLowerLimit);  // kuinka suurella tarkkuudella???
        case kFmiMaskGreaterThan:
          return (theValue > itsLowerLimit);
        case kFmiMaskLessThan:
          return (theValue < itsLowerLimit);
        case kFmiMaskGreaterOrEqualThan:
          return (theValue >= itsLowerLimit);
        case kFmiMaskLessOrEqualThan:
          return (theValue <= itsLowerLimit);
        case kFmiMaskGreaterThanAndLessThan:
          return (theValue > itsLowerLimit && theValue < itsUpperLimit);
        case kFmiMaskGreaterOrEqualAndLessOrEqual:
          return (theValue >= itsLowerLimit && theValue <= itsUpperLimit);
        case kFmiMaskGreaterOrEqualAndLessThan:
          return (theValue >= itsLowerLimit && theValue < itsUpperLimit);
        case kFmiMaskGreaterThanAndLessOrEqual:
          return (theValue > itsLowerLimit && theValue <= itsUpperLimit);
        case kFmiMaskLessThanOrGreaterThan:
          return (theValue < itsLowerLimit || theValue > itsUpperLimit);
        case kFmiMaskLessOrEqualOrGreaterOrEqual:
          return (theValue <= itsLowerLimit || theValue >= itsUpperLimit);
        case kFmiMaskLessOrEqualOrGreaterThan:
          return (theValue <= itsLowerLimit || theValue > itsUpperLimit);
        case kFmiMaskLessThanOrGreaterOrEqual:
          return (theValue < itsLowerLimit || theValue >= itsUpperLimit);
        case kFmiMaskEqualOrEqual:
          return (round(theValue) == itsLowerLimit || round(theValue) == itsUpperLimit);
        case kFmiMaskAll:
          return true;
        case kFmiMaskRisingRamp:
          return (theValue > itsLowerLimit);  // tähän ei ole oikeastaan kunnon tarkastelua,
                                              // todellinen käyttö kun lasketaan maskikertoimia!
        case kFmiMaskLoweringRamp:
          return (theValue < itsLowerLimit);  // tähän ei ole oikeastaan kunnon tarkastelua,
                                              // todellinen käyttö kun lasketaan maskikertoimia!
        case kFmiMaskDoubleRamp:
          return (theValue < itsLowerLimit);  // tähän ei ole oikeastaan kunnon tarkastelua,
                                              // todellinen käyttö kun lasketaan maskikertoimia!
        case kFmiMaskContinuousEqual:
          return false;  // requires two consecutive values; cannot evaluate from a single value
      }
    }

    return false;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

namespace
{
// Mask value for a boolean condition
double MaskResult(bool theCondition)
{
  return (theCondition ? 1 : 0);
}

// Linear ramp from 0 at the lower limit to 1 at the upper limit
double RisingRamp(double theValue, double theLowerLimit, double theUpperLimit)
{
  if (theValue <= theLowerLimit)
    return 0.;
  if (theValue > theLowerLimit && theValue <= theUpperLimit)
    return (theValue - theLowerLimit) / (theUpperLimit - theLowerLimit);
  return 1.;
}

// Linear ramp from 1 at the lower limit to 0 at the upper limit
double LoweringRamp(double theValue, double theLowerLimit, double theUpperLimit)
{
  if (theValue <= theLowerLimit)
    return 1.;
  if (theValue > theLowerLimit && theValue <= theUpperLimit)
    return 1. - (theValue - theLowerLimit) / (theUpperLimit - theLowerLimit);
  return 0.;
}

// Linear ramp from -1 at the lower limit to 1 at the upper limit
double DoubleRamp(double theValue, double theLowerLimit, double theUpperLimit)
{
  if (theValue <= theLowerLimit)
    return -1.;
  if (theValue > theLowerLimit && theValue < theUpperLimit)
    return 2 * ((theValue - theLowerLimit) / (theUpperLimit - theLowerLimit)) - 1;
  return 1.;
}
}  // namespace

// ----------------------------------------------------------------------
/*!
 * \param theValue Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// palauttaa kertoimen 0:n ja 1:n väliltä

double NFmiCalculationCondition::MaskValue(double theValue) const
{
  try
  {
    if (itsCondition == kFmiNoMaskOperation)
      return 1.;
    if (theValue == kFloatMissing)
    {
      return 0.;
    }

    switch (itsCondition)
    {
      case kFmiNoMaskOperation:
        return 1.;
      case kFmiMaskEqual:
        return MaskResult(round(theValue) == itsLowerLimit);  // kuinka suurella tarkkuudella???
      case kFmiMaskNotEqual:
        return MaskResult(round(theValue) != itsLowerLimit);  // kuinka suurella tarkkuudella???
      case kFmiMaskGreaterThan:
        return MaskResult(theValue > itsLowerLimit);
      case kFmiMaskLessThan:
        return MaskResult(theValue < itsLowerLimit);
      case kFmiMaskGreaterOrEqualThan:
        return MaskResult(theValue >= itsLowerLimit);
      case kFmiMaskLessOrEqualThan:
        return MaskResult(theValue <= itsLowerLimit);
      case kFmiMaskGreaterThanAndLessThan:
        return MaskResult(theValue > itsLowerLimit && theValue < itsUpperLimit);
      case kFmiMaskGreaterOrEqualAndLessOrEqual:
        return MaskResult(theValue >= itsLowerLimit && theValue <= itsUpperLimit);
      case kFmiMaskGreaterOrEqualAndLessThan:
        return MaskResult(theValue >= itsLowerLimit && theValue < itsUpperLimit);
      case kFmiMaskGreaterThanAndLessOrEqual:
        return MaskResult(theValue > itsLowerLimit && theValue <= itsUpperLimit);
      case kFmiMaskLessThanOrGreaterThan:
        return MaskResult(theValue < itsLowerLimit || theValue > itsUpperLimit);
      case kFmiMaskLessOrEqualOrGreaterOrEqual:
        return MaskResult(theValue <= itsLowerLimit || theValue >= itsUpperLimit);
      case kFmiMaskLessOrEqualOrGreaterThan:
        return MaskResult(theValue <= itsLowerLimit || theValue > itsUpperLimit);
      case kFmiMaskLessThanOrGreaterOrEqual:
        return MaskResult(theValue < itsLowerLimit || theValue >= itsUpperLimit);
      case kFmiMaskRisingRamp:
        return RisingRamp(theValue, itsLowerLimit, itsUpperLimit);
      case kFmiMaskLoweringRamp:
        return LoweringRamp(theValue, itsLowerLimit, itsUpperLimit);
      case kFmiMaskEqualOrEqual:
        return MaskResult(round(theValue) == itsLowerLimit || round(theValue) == itsUpperLimit);
      case kFmiMaskDoubleRamp:
        return DoubleRamp(theValue, itsLowerLimit, itsUpperLimit);
      case kFmiMaskAll:
        return 1;
      case kFmiMaskContinuousEqual:
        return 0.;  // requires two consecutive values; cannot evaluate from a single value
    }

    return 0.;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theParamName Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

// annetaan käytetyn parametrin nimi, valmistaa stringin, missä on ehdot näkyvissä

NFmiString NFmiCalculationCondition::MaskString(const NFmiString& theParamName) const
{
  try
  {
    NFmiString returnString;
    NFmiValueString lowerValue;
    NFmiValueString upperValue;
    returnString += theParamName;
    returnString += " ";
    lowerValue = NFmiValueString(itsLowerLimit, "%.1f");
    upperValue = NFmiValueString(itsUpperLimit, "%.1f");

    switch (itsCondition)
    {
      case kFmiNoMaskOperation:
        returnString += "-";
        break;
      case kFmiMaskEqual:
        returnString += "=";
        returnString += lowerValue;
        break;
      case kFmiMaskNotEqual:
        returnString += "!=";
        returnString += lowerValue;
        break;
      case kFmiMaskGreaterThan:
        returnString += ">";
        returnString += lowerValue;
        break;
      case kFmiMaskLessThan:
        returnString += "<";
        returnString += lowerValue;
        break;
      case kFmiMaskGreaterOrEqualThan:
        returnString += ">=";
        returnString += lowerValue;
        break;
      case kFmiMaskLessOrEqualThan:
        returnString += "<=";
        returnString += lowerValue;
        break;
      case kFmiMaskGreaterThanAndLessThan:
        returnString += lowerValue;
        returnString += "< x <";
        returnString += upperValue;
        break;
      case kFmiMaskGreaterOrEqualAndLessOrEqual:
        returnString += lowerValue;
        returnString += "<= x <=";
        returnString += upperValue;
        break;
      case kFmiMaskGreaterOrEqualAndLessThan:
        returnString += lowerValue;
        returnString += "<= x <";
        returnString += upperValue;
        break;
      case kFmiMaskGreaterThanAndLessOrEqual:
        returnString += lowerValue;
        returnString += "< x <=";
        returnString += upperValue;
        break;
      case kFmiMaskLessThanOrGreaterThan:
        returnString += lowerValue;
        returnString += "> x tai x >";
        returnString += upperValue;
        break;
      case kFmiMaskLessOrEqualOrGreaterOrEqual:
        returnString += lowerValue;
        returnString += ">= x tai x >=";
        returnString += upperValue;
        break;
      case kFmiMaskLessOrEqualOrGreaterThan:
        returnString += lowerValue;
        returnString += ">= x tai x >";
        returnString += upperValue;
        break;
      case kFmiMaskLessThanOrGreaterOrEqual:
        returnString += lowerValue;
        returnString += "> x tai x >=";
        returnString += upperValue;
        break;
      case kFmiMaskAll:
        returnString += "*";
        break;
      case kFmiMaskEqualOrEqual:
        returnString += "x=";
        returnString += lowerValue;
        returnString += " tai x=";
        returnString += upperValue;
        break;
      case kFmiMaskRisingRamp:
        returnString += " nouseva R";
        break;
      case kFmiMaskLoweringRamp:
        returnString += " laskeva R";
        break;
      case kFmiMaskDoubleRamp:
        returnString += " tupla R";
        break;
      case kFmiMaskContinuousEqual:
        returnString += " ~=";
        break;
    }
    return returnString;
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiCalculationCondition::Write(std::ostream& os) const
{
  try
  {
    os << static_cast<int>(itsCondition) << ' ' << itsLowerLimit << ' ' << itsUpperLimit
       << '\n';
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

void NFmiCalculationCondition::Read(std::istream& is)
{
  try
  {
    int tmp = 0;
    is >> tmp >> itsLowerLimit >> itsUpperLimit;
    itsCondition = static_cast<FmiMaskOperation>(tmp);
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Operation failed!");
  }
}

// ======================================================================
