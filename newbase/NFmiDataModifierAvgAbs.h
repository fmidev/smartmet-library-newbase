// ======================================================================
/*!
 * \file NFmiDataModifierAvgAbs.h
 * \brief Interface of class NFmiDataModifierAvgAbs
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class NFmiDataModifierAvgAbs : public NFmiDataModifier
{
 public:
  ~NFmiDataModifierAvgAbs() override;
  NFmiDataModifierAvgAbs();

  void Calculate(float theValue) override;
  void Calculate(NFmiQueryInfo* theQI) override;

  void Clear() override;
  using NFmiDataModifier::CalculationResult;
  float CalculationResult() override;
  long Counter() const;

 protected:
  long itsCounter = 0;
  float itsSum = 0;
  float itsAverage = 0.0F;

};  // class NFmiDataModifierAvgAbs

// ======================================================================
