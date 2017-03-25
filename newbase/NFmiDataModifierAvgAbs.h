// ======================================================================
/*!
 * \file NFmiDataModifierAvgAbs.h
 * \brief Interface of class NFmiDataModifierAvgAbs
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierAvgAbs : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierAvgAbs() override;
  NFmiDataModifierAvgAbs();

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  long Counter();

 protected:
  long itsCounter;
  float itsSum;
  float itsAverage;

};  // class NFmiDataModifierAvgAbs

// ======================================================================
