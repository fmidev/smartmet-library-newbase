// ======================================================================
/*!
 * \file NFmiDataModifierMinMax.h
 * \brief Interface of class NFmiDataModifierMinMax
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

// Mika: For some reason Red-Hat Linux does not have FLT_MAX...?

#ifdef UNIX
#include <limits.h>
#ifndef FLT_MAX
#define FLT_MAX 1e+37
#endif
#else
#include <limits>
#endif

class _FMI_DLL NFmiDataModifierMinMax : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierMinMax(void) override;
  NFmiDataModifierMinMax(void);
  NFmiDataModifierMinMax(const NFmiDataModifierMinMax& theOther);
  NFmiDataModifier* Clone(void) const override;

  float MinValue(void);
  float MaxValue(void);

  void Clear(void) override;

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult(void) override;  // kysy erikseen min ja max!

 private:
  float itsMinValue;
  float itsMaxValue;

};  // class NFmiDataModifierMinMax

// ======================================================================
