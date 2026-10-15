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
  NFmiDataModifierAvg& operator=(const NFmiDataModifierAvg&) = delete;
  NFmiDataModifier* Clone() const override;

  float Avg() const;
  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  long Counter() const;

 protected:
  long itsCounter = 0;
  float itsAverage = 0.;

};  // class NFmiDataModifierAvg

// ======================================================================
