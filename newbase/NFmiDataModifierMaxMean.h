// ======================================================================
/*!
 * \file NFmiDataModifierMaxMean.h
 * \brief Interface of class NFmiDataModifierMaxMean
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierMaxMean : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierMaxMean(void) override;
  NFmiDataModifierMaxMean(void);
  NFmiDataModifierMaxMean(float theWeightFactor);

  void Clear(void) override;

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue) override;
  float CalculationResult(void) override;

 private:
  long itsCounter;
  float itsAverage;
  float itsMaxValue;
  float itsWeightFactor;

};  // class NFmiDataModifierMaxMean

// ======================================================================
