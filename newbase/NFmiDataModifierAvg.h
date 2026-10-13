// ======================================================================
/*!
 * \file NFmiDataModifierAvg.h
 * \brief Interface of class NFmiDataModifierAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierAvg : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierAvg() override;
  NFmiDataModifierAvg();
  NFmiDataModifierAvg(const NFmiDataModifierAvg& theOther);
  NFmiDataModifier* Clone() const override;

  float Avg() const;
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  long Counter() const;

 protected:
  long itsCounter;
  float itsAverage;

};  // class NFmiDataModifierAvg

// ======================================================================
