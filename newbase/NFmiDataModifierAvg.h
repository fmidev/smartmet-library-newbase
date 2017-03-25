// ======================================================================
/*!
 * \file NFmiDataModifierAvg.h
 * \brief Interface of class NFmiDataModifierAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierAvg : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierAvg(void) override;
  NFmiDataModifierAvg(void);
  NFmiDataModifierAvg(const NFmiDataModifierAvg& theOther);
  NFmiDataModifier* Clone(void) const override;

  float Avg(void);
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear(void) override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void) override;
  long Counter(void);

 protected:
  long itsCounter;
  float itsAverage;

};  // class NFmiDataModifierAvg

// ======================================================================
