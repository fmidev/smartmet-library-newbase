// ======================================================================
/*!
 * \file NFmiDataModifierSingleExtremeValuesFilter.h
 * \brief Interface of class NFmiDataModifierSingleExtremeValuesFilter
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class _FMI_DLL NFmiDataModifierSingleExtremeValuesFilter : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierSingleExtremeValuesFilter(void) override;
  NFmiDataModifierSingleExtremeValuesFilter(void);
  NFmiDataModifierSingleExtremeValuesFilter(float thePointValue, float theLimit);
  void Clear(void) override;
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult(void) override;

 private:
  NFmiDataModifierAvg itsAvgModifier;
  float itsValue;
  float itsLimit;

};  // class NFmiDataModifierSingleExtremeValuesFilter

// ======================================================================
