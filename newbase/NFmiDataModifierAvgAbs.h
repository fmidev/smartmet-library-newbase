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
  ~NFmiDataModifierAvgAbs(void) override;
  NFmiDataModifierAvgAbs(void);

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear(void) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;
  long Counter(void);

 protected:
  long itsCounter;
  float itsSum;
  float itsAverage;

};  // class NFmiDataModifierAvgAbs

// ======================================================================
